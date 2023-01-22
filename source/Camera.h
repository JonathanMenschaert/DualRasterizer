#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{
		}


		Vector3 origin{};
		float fovAngle{ 90.f };
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};

		Matrix projectionMatrix{};

		float nearPlane{ 0.1f };
		float farPlane{ 100.f };
		float aspectRatio{};

		const float maxPitch{ 89.99f * TO_RADIANS };
		const float maxYaw{ 360.f * TO_RADIANS };

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = { 0.f,0.f,0.f }, float ar = 1.7f, float zn = 0.1f, float zf = 100.f)
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);
			aspectRatio = ar;
			nearPlane = zn;
			farPlane = zf;
			origin = _origin;
			CalculateProjectionMatrix();
		}

		const Matrix& GetViewMatrix() const
		{
			return viewMatrix;
		}

		const Matrix& GetInvViewMatrix() const
		{
			return invViewMatrix;
		}

		const Matrix& GetProjectionMatrix() const
		{
			return projectionMatrix;
		}

		void CalculateViewMatrix()
		{
			//TODO W1
			//ONB => invViewMatrix
			//Inverse(ONB) => ViewMatrix
			const Matrix finalRotation = Matrix::CreateRotation({ totalPitch, totalYaw, 0.f });
			forward = finalRotation.TransformVector(Vector3::UnitZ);
			forward.Normalize();
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();
			invViewMatrix = { right, up, forward, origin };

			viewMatrix = Matrix::Inverse(invViewMatrix);
		}

		void CalculateProjectionMatrix()
		{

			projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane);
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void Update(const Timer* pTimer)
		{
			const float deltaTime{ pTimer->GetElapsed() };

			//Camera Update Logic
			const float linearSpeed{ 30.f };
			const float rotationSpeed {150.f * TO_RADIANS};
			const float mouseSensivity{ 1.5f};

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			const bool isShiftPressed{ pKeyboardState[SDL_SCANCODE_LSHIFT] || pKeyboardState[SDL_SCANCODE_RSHIFT] };
			const float shiftModifier{ 3.f * isShiftPressed + 1.f};
			const float speedModifier{ deltaTime * linearSpeed * shiftModifier };

			const bool isForwardsPressed{ pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP] };
			const bool isBackwardsPressed{ pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN] };
			const bool isRightPressed{ pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT] };
			const bool isLeftPressed{ pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT] };

			//Calculate camera origin position
			origin += forward * speedModifier * isForwardsPressed;
			origin += forward * -speedModifier * isBackwardsPressed;
			origin += right * speedModifier * isRightPressed;
			origin += right * -speedModifier * isLeftPressed;

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			const float rotationModifier{rotationSpeed * shiftModifier * deltaTime};

			switch (mouseState)
			{
			case SDL_BUTTON_X2:
				origin -= Vector3::UnitY * static_cast<float>(mouseY) * mouseSensivity * shiftModifier;
				break;
			case SDL_BUTTON_LMASK:
				origin -= forward * static_cast<float>(mouseY) * mouseSensivity ;
				totalYaw += static_cast<float>(mouseX) * rotationModifier;
				break;
			case SDL_BUTTON_RMASK:
				totalYaw += static_cast<float>(mouseX) * rotationModifier;
				totalPitch -= static_cast<float>(mouseY) * rotationModifier;
				break;
			default:
				break;
			}

			//Clamp Pitch between -90° and 90°
			totalPitch = std::clamp(totalPitch, -maxPitch, maxPitch);

			//Update Matrices
			CalculateViewMatrix();
		}
	};
}


