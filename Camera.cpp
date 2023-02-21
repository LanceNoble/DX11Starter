#include "Camera.h"
#include "Input.h"

using namespace DirectX;

/// <summary>
/// Initialize the properties of the Camera object that's about to be created
/// </summary>
/// <param name="aspRat">The aspect ratio of the Camera object</param>
/// <param name="initPos">The initial position of the Camera object</param>
/// <param name="fov">The field of vision of the Camera objecy</param>
Camera::Camera(float aspRat, XMFLOAT3 initPos, float fov)
{
	this->aspRat = aspRat;
	this->initPos = initPos;
	transform.SetPosition(this->initPos);
	XMVECTOR initRotCopy = XMLoadFloat4(&initRot);
	initRotCopy = XMQuaternionIdentity();
	XMStoreFloat4(&initRot, initRotCopy);
	this->fov = fov;
	nearClip = 0.1f;
	farClip = 900;
	moveSp = 1;
	lookSp = 0.001f;
	isPerspective = true;

	UpdateViewMatrix();
	UpdateProjectionMatrix(aspRat);
}

/// <summary>
/// get cam's view matrix
/// </summary>
/// <returns>cam's view matrix</returns>
XMFLOAT4X4 Camera::GetViewMat()
{
	return viewMat;
}

/// <summary>
/// get cam's projection matrix
/// </summary>
/// <returns>cam's projection matrix</returns>
XMFLOAT4X4 Camera::GetProjMat()
{
	return projMat;
}

/// <summary>
/// update cam's projection matrix
/// </summary>
/// <param name="aspRat">aspect ratio of cam's new projection matrix</param>
void Camera::UpdateProjectionMatrix(float aspRat)
{
	this->aspRat = aspRat;
	XMMATRIX projMatCopy = XMLoadFloat4x4(&projMat);
	projMatCopy = XMMatrixPerspectiveFovLH(fov, aspRat, nearClip, farClip);
	XMStoreFloat4x4(&projMat, projMatCopy);
}

/// <summary>
/// update cam's view matrix
/// </summary>
void Camera::UpdateViewMatrix()
{
	XMFLOAT3 pos = transform.GetPosition();
	XMVECTOR position = XMLoadFloat3(&pos);
	XMFLOAT3 dir = transform.GetForward();
	XMVECTOR direction = XMLoadFloat3(&dir);
	XMFLOAT3 up = XMFLOAT3(0, 1, 0);
	XMVECTOR upCopy = XMLoadFloat3(&up);
	XMMATRIX viewMatCopy = XMLoadFloat4x4(&viewMat);
	viewMatCopy = XMMatrixLookToLH(position, direction, upCopy);
	XMStoreFloat4x4(&viewMat, viewMatCopy);
}

/// <summary>
/// update cam's position
/// </summary>
/// <param name="dt">delta time</param>
void Camera::Update(float dt)
{
	Input& input = Input::GetInstance();

	if (input.KeyDown('W'))
	{
		transform.MoveRelative(0, 0, moveSp * dt);
	}
	if (input.KeyDown('S'))
	{
		transform.MoveRelative(0, 0, moveSp * dt * -1);
	}
	if (input.KeyDown('A'))
	{
		//transform.MoveRelative(moveSp * dt * -1, 0, 0);
		XMFLOAT3 offset = XMFLOAT3();
		XMFLOAT3 up = XMFLOAT3(0,1,0);
		XMVECTOR upCopy = XMLoadFloat3(&up);
		XMFLOAT3 forward = transform.GetForward();
		XMVECTOR forwardCopy = XMLoadFloat3(&forward);
		XMVECTOR offsetCopy = XMVector3Cross(upCopy, forwardCopy);
		offsetCopy = offsetCopy / -999;
		XMStoreFloat3(&offset, offsetCopy);
		transform.MoveAbsolute(offset);
	}
	if (input.KeyDown('D'))
	{
		//transform.MoveRelative(moveSp * dt, 0, 0);
		XMFLOAT3 offset = XMFLOAT3();
		XMFLOAT3 up = XMFLOAT3(0, 1, 0);
		XMVECTOR upCopy = XMLoadFloat3(&up);
		XMFLOAT3 forward = transform.GetForward();
		XMVECTOR forwardCopy = XMLoadFloat3(&forward);
		XMVECTOR offsetCopy = XMVector3Cross(upCopy, forwardCopy);
		offsetCopy = offsetCopy / 999;
		XMStoreFloat3(&offset, offsetCopy);
		transform.MoveAbsolute(offset);
	}
	if (input.KeyDown(' '))
	{
		transform.MoveAbsolute(0, moveSp * dt, 0);
	}
	if (input.KeyDown('X'))
	{
		transform.MoveAbsolute(0, moveSp * dt * -1, 0);
	}
	if (input.MouseLeftDown())
	{
		float cursorMovementX = input.GetMouseXDelta() * lookSp;
		float cursorMovementY = input.GetMouseYDelta() * lookSp;

		if (cursorMovementY > -1.57 && cursorMovementY < 1.57)
		{
			transform.Rotate(0, cursorMovementX, 0);
		}
		transform.Rotate(cursorMovementY, 0, 0);
	}
	UpdateViewMatrix();
}
