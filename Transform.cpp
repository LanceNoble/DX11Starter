#include "Transform.h"
#include <iostream>
using namespace DirectX;


// Note: when using quaternions to calculate rotations, always normalize them, it's more efficient

/// <summary>
/// Set default values for the entity's transform
/// </summary>
Transform::Transform() {
	position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	XMStoreFloat4x4(&world, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixIdentity());
	XMStoreFloat4(&orientation, XMQuaternionIdentity());
}

/// <summary>
/// Set the position of the entity's transform
/// </summary>
/// <param name="x">The x coordinate of the new position</param>
/// <param name="y">The y coordinate of the new position</param>
/// <param name="z">The z coordinate of the new position</param>
void Transform::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
}

/// <summary>
/// Set the position of the entity's transform
/// </summary>
/// <param name="position">The new position</param>
void Transform::SetPosition(DirectX::XMFLOAT3 position)
{
	this->position = position;
}

/// <summary>
/// Set the orientation of the entity's transform
/// </summary>
/// <param name="pitch">the pitch of the new position</param>
/// <param name="yaw">the yaw of the new position</param>
/// <param name="roll">the roll of the new position</param>
void Transform::SetOrientation(float pitch, float yaw, float roll)
{
	XMVECTOR quat = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
	quat = XMQuaternionNormalize(quat);
	XMStoreFloat4(&orientation, quat);
	//XMVECTOR orientationCopy = XMLoadFloat4(&orientation);
	//XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
	//orientationCopy = XMQuaternionMultiply(orientationCopy, rotQuat);
	//XMStoreFloat4(&orientation, orientationCopy);

}

/// <summary>
/// Set the orientation of the entity's transform
/// </summary>
/// <param name="orientation">The new orientation</param>
void Transform::SetOrientation(DirectX::XMFLOAT4 orientation)
{
	XMVECTOR orientationCopy = XMLoadFloat4(&orientation);
	orientationCopy = XMQuaternionNormalize(orientationCopy);
	XMStoreFloat4(&this->orientation, orientationCopy);
}

/// <summary>
/// Set the scale of the entity's transform
/// </summary>
/// <param name="x">The x value of the new scale</param>
/// <param name="y">The y value of the new scale</param>
/// <param name="z">The z value of the new scale</param>
void Transform::SetScale(float x, float y, float z)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;
}

/// <summary>
/// Set the scale of the entity's transform
/// </summary>
/// <param name="scale">The new scale</param>
void Transform::SetScale(DirectX::XMFLOAT3 scale)
{
	this->scale = scale;
}

/// <summary>
/// Get the entity's position
/// </summary>
/// <returns>the entity's position</returns>
DirectX::XMFLOAT3 Transform::GetPosition()
{
	return position;
}

/// <summary>
/// Get the entity's orientation
/// </summary>
/// <returns>the entity's orientation</returns>
DirectX::XMFLOAT4 Transform::GetOrientation()
{
	return orientation;
}

/// <summary>
/// Get the entity's scale
/// </summary>
/// <returns>the entity's scale</returns>
DirectX::XMFLOAT3 Transform::GetScale()
{
	return scale;
}

/// <summary>
/// Get the entity's world matrix
/// </summary>
/// <returns>the entity's world matrix</returns>
DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	return world;
}

/// <summary>
/// get the inverse transpose version of the entity's world matrix
/// </summary>
/// <returns>the inverse transpose version of the entity's world matrix</returns>
DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	return worldInverseTranspose;
}

/// <summary>
/// move the entity globally
/// </summary>
/// <param name="x">how much to move the entity by in the x direction</param>
/// <param name="y">how much to move the entity by in the y direction</param>
/// <param name="z">how much to move the entity by in the z direction</param>
void Transform::MoveAbsolute(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
}

/// <summary>
/// move the entity globally
/// </summary>
/// <param name="offset">how much to move the entity by</param>
void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
	XMVECTOR posCopy = XMLoadFloat3(&position);
	XMVECTOR offCopy = XMLoadFloat3(&offset);
	posCopy = XMVectorAdd(posCopy, offCopy);
	XMStoreFloat3(&position, posCopy);
}

/// <summary>
/// move the entity based on its orientation
/// </summary>
/// <param name="x">how far to move in the x direction</param>
/// <param name="y">how far to move in the y direction</param>
/// <param name="z">how far to move in the z direction</param>
void Transform::MoveRelative(float x, float y, float z)
{
	XMFLOAT3 eulers = XMFLOAT3(x, y, z);
	XMVECTOR eulersCopy = XMLoadFloat3(&eulers);
	XMVECTOR quat = XMLoadFloat4(&this->orientation);
	XMVECTOR eulersRotated = XMVector3Rotate(eulersCopy, quat);
	XMVECTOR position = XMLoadFloat3(&this->position);
	position = XMVectorAdd(eulersRotated, position);
	//position += eulersRotated;
	XMStoreFloat3(&this->position, position);
}

/// <summary>
/// rotate entity
/// </summary>
/// <param name="pitch">how much to rotate the entity by in the x direction</param>
/// <param name="yaw">how much to rotate the entity by in the y direction</param>
/// <param name="roll">how much to rotate the entity by in the z direction</param>
void Transform::Rotate(float pitch, float yaw, float roll)
{
	XMVECTOR orientationCopy = XMLoadFloat4(&orientation);
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
	rotQuat = XMQuaternionNormalize(rotQuat);
	orientationCopy = XMQuaternionMultiply(orientationCopy, rotQuat);
	XMStoreFloat4(&orientation, orientationCopy);
}

/// <summary>
/// rotate entity
/// </summary>
/// <param name="rotation">how much to rotate the entity by</param>
void Transform::Rotate(DirectX::XMFLOAT3 rotation)
{
	XMVECTOR orientationCopy = XMLoadFloat4(&orientation);
	XMVECTOR rotQuat = XMLoadFloat3(&rotation);
	rotQuat = XMQuaternionRotationRollPitchYawFromVector(rotQuat);
	rotQuat = XMQuaternionNormalize(rotQuat);
	orientationCopy = XMQuaternionMultiply(orientationCopy, rotQuat);
	XMStoreFloat4(&orientation, orientationCopy);
}

/// <summary>
/// scale entity
/// </summary>
/// <param name="x">how much to scale the entity by in the x direction</param>
/// <param name="y">how much to scale the entity by in the y direction</param>
/// <param name="z">how much to scale the entity by in the z direction</param>
void Transform::Scale(float x, float y, float z)
{
	scale.x += x;
	scale.y += y;
	scale.z += z;
}

/// <summary>
/// scale entity
/// </summary>
/// <param name="scalar">how much to scale the entity by</param>
void Transform::Scale(DirectX::XMFLOAT3 scalar)
{
	XMVECTOR scaleCopy = XMLoadFloat3(&scale);
	XMVECTOR scalarCopy = XMLoadFloat3(&scalar);
	scaleCopy = XMVectorAdd(scaleCopy, scalarCopy);
	XMStoreFloat3(&position, scaleCopy);
}

/// <summary>
/// rotate the world's right vector
/// </summary>
/// <returns>new right vector</returns>
XMFLOAT3 Transform::GetRight()
{
	XMFLOAT3 eulers = XMFLOAT3(1, 0, 0);
	XMVECTOR eulersCopy = XMLoadFloat3(&eulers);
	XMVECTOR quat = XMLoadFloat4(&this->orientation);
	XMVECTOR eulersRotated = XMVector3Rotate(eulersCopy, quat);
	XMFLOAT3 finalEulers;
	XMStoreFloat3(&finalEulers, eulersRotated);
	return finalEulers;
}

/// <summary>
/// rotate the world's up vector
/// </summary>
/// <returns>new up vector</returns>
XMFLOAT3 Transform::GetUp()
{
	XMFLOAT3 eulers = XMFLOAT3(0, 1, 0);
	XMVECTOR eulersCopy = XMLoadFloat3(&eulers);
	XMVECTOR quat = XMLoadFloat4(&this->orientation);
	XMVECTOR eulersRotated = XMVector3Rotate(eulersCopy, quat);
	XMFLOAT3 finalEulers;
	XMStoreFloat3(&finalEulers, eulersRotated);
	return finalEulers;
}

/// <summary>
/// rotate the world's forward vector
/// </summary>
/// <returns>new forward vector</returns>
XMFLOAT3 Transform::GetForward()
{
	XMFLOAT3 eulers = XMFLOAT3(0, 0, 1);
	XMVECTOR eulersCopy = XMLoadFloat3(&eulers);
	XMVECTOR quat = XMLoadFloat4(&this->orientation);
	XMVECTOR eulersRotated = XMVector3Rotate(eulersCopy, quat);
	XMFLOAT3 finalEulers;
	XMStoreFloat3(&finalEulers, eulersRotated);
	return finalEulers;
}

/// <summary>
/// update entity's world matrix
/// </summary>
void Transform::UpdateMatrices()
{
	XMMATRIX translation = XMMatrixTranslation(position.x, position.y, position.z);
	XMVECTOR orientationCopy = XMLoadFloat4(&orientation);
	XMMATRIX rotation = XMMatrixRotationQuaternion(orientationCopy);//XMMatrixRotationQuaternion(orientationCopy);
	XMMATRIX scale = XMMatrixScaling(this->scale.x, this->scale.y, this->scale.z);

	// Overloaded operators are defined in the DirectX namespace
	// Alternatively, you can call XMMatrixMultiply(XMMatrixMultiply(s,r), t)
	// Generally, we t * r * s
	// But here we do reverse order because the matrix we send to the shader will be transposed
	// This is because DirectX does row major storage but hlsl uses column major
	// this reverse order is to account for that transpose
	XMMATRIX world = scale * rotation * translation;

	XMStoreFloat4x4(&this->world, world);
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixInverse(0, XMMatrixTranspose(world)));
}
