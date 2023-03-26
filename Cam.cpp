#include "Cam.h"
#include "Input.h"

using namespace DirectX;

/// <summary>
/// Set this cam's properties
/// </summary>
/// <param name="aspRat">- aspect ratio</param>
/// <param name="initPos">- initial position in world</param>
/// <param name="initOri">- initial orientation in world</param>
/// <param name="fov">- field of view</param>
/// <param name="nearClip">- how far into the world this cam starts seeing</param>
/// <param name="farClip">- how far into the world this cam stops seeing</param>
/// <param name="moveSp">- how fast this cam moves in the world</param>
/// <param name="lookSpX">- how fast this cam turns left/right</param>
/// <param name="lookSpY">- how fast this cam turns up/down</param>
Cam::Cam(float aspRat, XMFLOAT3 initPos, XMFLOAT3 initOri, float fov, float nearClip, float farClip, float moveSp, float lookSpX, float lookSpY)
{
	tf.SetPosition(initPos.x, initPos.y, initPos.z);
	tf.SetOrientation(initOri.x, initOri.y, initOri.z);
	this->aspRat = aspRat;
	this->fov = fov;
	this->nearClip = nearClip;
	this->farClip = farClip;
	this->moveSp = moveSp;
	this->lookSpX = lookSpX;
	this->lookSpY = lookSpY;
	
	// So we don't have to resize the window when the program starts
	UpdateProj(aspRat);
}

/// <returns>This cam's view matrix</returns>
XMFLOAT4X4 Cam::GetView()
{
	return view;
}

/// <returns>This cam's projection matrix</returns>
XMFLOAT4X4 Cam::GetProj()
{
	return proj;
}

/// <returns>This cam's position in the world</returns>
XMFLOAT3 Cam::GetPos()
{
	return tf.GetPosition();
}

/// <returns>This cam's field of view</returns>
float Cam::GetFOV()
{
	return fov;	
}

/// <returns>This cam's aspect ratio</returns>
float Cam::GetAspRat()
{
	return aspRat;
}

/// <summary>
/// Update the aspect ratio of this cam's projection matrix
/// </summary>
/// <param name="aspRat">- The new aspect ratio of this cam's projection matrix</param>
void Cam::UpdateProj(float aspRat)
{
	this->aspRat = aspRat;
	XMStoreFloat4x4(&proj, XMMatrixPerspectiveFovLH(fov, aspRat, nearClip, farClip));
}

/// <summary>
/// <para>Update this cam's position in the world</para>
/// <para>Thinking of moving this move code to the Transform class if I need multiple cams, </para>
/// <para>or cams that aren't tied to user input</para>
/// </summary>
/// <param name="dt">- deltaTime: the time taken to get from one frame to the next</param>
void Cam::Move(float dt)
{
	Input& input = Input::GetInstance();

	// Putting all these inputs in if statements allows for movement in multiple directions simultaneously
	if (input.KeyDown('W')) tf.MoveRelative(0, 0, moveSp * dt);
	if (input.KeyDown('S')) tf.MoveRelative(0, 0, moveSp * dt * -1);
	if (input.KeyDown('A')) tf.MoveRelative(moveSp * dt * -1, 0, 0);
	if (input.KeyDown('D')) tf.MoveRelative(moveSp * dt, 0, 0);
	if (input.KeyDown('E')) tf.MoveAbsolute(0, moveSp * dt, 0);
	if (input.KeyDown('Q')) tf.MoveAbsolute(0, moveSp * dt * -1, 0);
	if (input.MouseLeftDown())
	{
		float cursorMovementX = input.GetMouseXDelta() * lookSpX;
		float cursorMovementY = input.GetMouseYDelta() * lookSpY;

		// transformCopy rotates first to check if it's safe for the real transform to rotate
		// If we use the real transform to check, the camera will get stuck
		Transform tfCopy = tf;
		tfCopy.RotAx(tf.GetRight(), cursorMovementY);
		XMFLOAT3 forward = tfCopy.GetForward();
		XMVECTOR forwardMath = XMLoadFloat3(&forward);
		XMFLOAT3 worldUp = XMFLOAT3(0, 1, 0);
		XMVECTOR worldUpMath = XMLoadFloat3(&worldUp);
		XMVECTOR angleBtwnMath = XMVector3AngleBetweenVectors(forwardMath, worldUpMath);
		XMFLOAT3 angleBtwn;
		XMStoreFloat3(&angleBtwn, angleBtwnMath);
		
		// I like to work with degrees and convert them to radians when necessary.
		// It's probably more optimal to skip the conversion and just put the rad value directly.
		if (angleBtwn.y < XMConvertToRadians(170) && angleBtwn.y > XMConvertToRadians(10))
			tf.RotAx(tf.GetRight(), cursorMovementY);

		tf.RotAx(XMFLOAT3(0, 1, 0), cursorMovementX);
	}

	// Update this cam's view matrix
	XMFLOAT3 pos = tf.GetPosition();
	XMVECTOR posMath = XMLoadFloat3(&pos);
	XMFLOAT3 dir = tf.GetForward();
	XMVECTOR dirMath = XMLoadFloat3(&dir);
	XMFLOAT3 up = XMFLOAT3(0, 1, 0);
	XMVECTOR upMath = XMLoadFloat3(&up);
	XMStoreFloat4x4(&view, XMMatrixLookToLH(posMath, dirMath, upMath));
}
