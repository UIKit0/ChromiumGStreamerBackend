// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "mash/quick_launch/quick_launch.h"
#include "services/shell/public/c/main.h"
#include "services/shell/public/cpp/service_runner.h"

MojoResult ServiceMain(MojoHandle service_request_handle) {
  shell::ServiceRunner runner(new mash::quick_launch::QuickLaunch);
  return runner.Run(service_request_handle);
}
