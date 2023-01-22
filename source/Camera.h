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
			forward = finalRotation.TransformVector(Vector3::UnitZ).Normalized();
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
			const float deltaTime = pTimer->GetElapsed();

			//Camera Update Logic
			const float linearSpeed{ 20.f };
			const float rotationSpeed{ 360.f * TO_RADIANS};

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			const bool isShiftPressed{ pKeyboardState[SDL_SCANCODE_LSHIFT] || pKeyboardState[SDL_SCANCODE_RSHIFT] };
			const float shiftModifier{ 3.f * isShiftPressed + 1.f};
			const float speedModifier{ deltaTime * linearSpeed * shiftModifier };

			const bool isForwardsPressed{ pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP] };
			const bool isBackwardsPressed{ pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN] };
			const bool isRightPressed{ pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT] };
			const bool isLeftPressed{ pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT] };
			origin += forward * speedModifier * isForwardsPressed;
			origin += forward * -speedModifier * isBackwardsPressed;
			origin += right * speedModifier * isRightPressed;
			origin += right * -speedModifier * isLeftPressed;

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			const float rotationModifier{ deltaTime * rotationSpeed * shiftModifier };

			//Calculate rotation & movement on mouse movement
			SDL_BUTTON_X2;
			if (mouseY != 0.f || mouseX != 0.f)
			{
				//Invert mouse Y
				mouseY *= -1;

				Vector3 forwardSpeed = forward * speedModifier * (mouseState == SDL_BUTTON_LMASK) * static_cast<float>(mouseY);
				origin += forwardSpeed;
				origin += Vector3::UnitY * speedModifier * (mouseState == (SDL_BUTTON_RMASK | SDL_BUTTON_LMASK)) * static_cast<float>(mouseY);
				totalPitch += static_cast<float>(mouseY) * (mouseState == SDL_BUTTON_RMASK) * rotationModifier;
				float difference = static_cast<float>(mouseX) * (mouseState & SDL_BUTTON_LMASK || mouseState & SDL_BUTTON_RMASK) * rotationModifier;
				//std::cout << "Speed Pitch: " << difference << "\n";
				totalYaw += difference;
			}

			//Update Matrices
			CalculateViewMatrix();
		}
	};
}


