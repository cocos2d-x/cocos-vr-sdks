/****************************************************************************
 Copyright (c) 2016 Chukong Technologies Inc.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "platform/CCPlatformMacros.h"
#include "CCVRGearVRHeadTracker.h"


NS_CC_BEGIN

VRGearVRHeadTracker::VRGearVRHeadTracker()
    : _ovr(nullptr)
{
}

VRGearVRHeadTracker::~VRGearVRHeadTracker()
{
}

Vec3 VRGearVRHeadTracker::getLocalPosition()
{
    if (!_ovr) return Vec3::ZERO;
    return Vec3(_tracking.HeadPose.Pose.Position.x, _tracking.HeadPose.Pose.Position.y, _tracking.HeadPose.Pose.Position.z);
}

Mat4 VRGearVRHeadTracker::getLocalRotation()
{
    if (!_ovr) return Mat4::IDENTITY;
    auto rot = _tracking.HeadPose.Pose.Orientation;
    Mat4 rotMat;
    Mat4::createRotation(Quaternion(rot.x, rot.y, rot.z, rot.w), &rotMat);
    return rotMat;
}

void VRGearVRHeadTracker::applyTracking(double predictedDisplayTime)
{
    if (!_ovr) return;
    const ovrTracking baseTracking = vrapi_GetPredictedTracking(_ovr, predictedDisplayTime);
    const ovrHeadModelParms headModelParms = vrapi_DefaultHeadModelParms();
    _tracking = vrapi_ApplyHeadModel(&headModelParms, &baseTracking);
}

NS_CC_END