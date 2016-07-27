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
#include "CCVROculusHeadTracker.h"


NS_CC_BEGIN

VROculusHeadTracker::VROculusHeadTracker()
    : _HMD(nullptr)
{
}

VROculusHeadTracker::~VROculusHeadTracker()
{
}

Vec3 VROculusHeadTracker::getLocalPosition()
{
    if (!_HMD) return Vec3::ZERO;
    return Vec3(_tracking.HeadPose.ThePose.Position.x, _tracking.HeadPose.ThePose.Position.y, _tracking.HeadPose.ThePose.Position.z);
}

Mat4 VROculusHeadTracker::getLocalRotation()
{
    if (!_HMD) return Mat4::IDENTITY;
    Mat4 rotMat;
    Mat4::createRotation(Quaternion(_tracking.HeadPose.ThePose.Orientation.x, _tracking.HeadPose.ThePose.Orientation.y, _tracking.HeadPose.ThePose.Orientation.z, _tracking.HeadPose.ThePose.Orientation.w), &rotMat);
    return rotMat;
}

void cocos2d::VROculusHeadTracker::applyTracking(double predictedDisplayTime)
{
    if (!_HMD) return;
    _tracking = ovr_GetTrackingState(_HMD, predictedDisplayTime, ovrTrue);
}

NS_CC_END