// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_PHYSICAL_WEB_CREATE_PHYSICAL_WEB_DATA_SOURCE_H_
#define IOS_CHROME_BROWSER_PHYSICAL_WEB_CREATE_PHYSICAL_WEB_DATA_SOURCE_H_

#include <memory>

class PhysicalWebDataSource;

// Creates a new instance of IOSChromePhysicalWebDataSource. The returned object
// is fully initialized and can be registered as global singleton.
std::unique_ptr<PhysicalWebDataSource> CreateIOSChromePhysicalWebDataSource();

#endif  // IOS_CHROME_BROWSER_PHYSICAL_WEB_CREATE_PHYSICAL_WEB_DATA_SOURCE_H_
