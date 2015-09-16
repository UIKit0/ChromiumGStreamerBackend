// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_STORAGE_DURABLE_STORAGE_PERMISSION_INFOBAR_DELEGATE_H_
#define CHROME_BROWSER_STORAGE_DURABLE_STORAGE_PERMISSION_INFOBAR_DELEGATE_H_

#include <string>
#include "chrome/browser/permissions/permission_infobar_delegate.h"
#include "components/content_settings/core/common/content_settings_types.h"

class GURL;
class InfoBarService;

class DurableStoragePermissionInfoBarDelegate
    : public PermissionInfobarDelegate {
 public:
  // Creates a DurableStorage permission infobar and delegate and adds the
  // infobar to
  // |infobar_service|.  Returns the infobar if it was successfully added.
  static infobars::InfoBar* Create(
      InfoBarService* infobar_service,
      const GURL& requesting_frame,
      const std::string& display_languages,
      ContentSettingsType type,
      const PermissionSetCallback& callback);

 private:
  DurableStoragePermissionInfoBarDelegate(
      const GURL& requesting_frame,
      const std::string& display_languages,
      ContentSettingsType type,
      const PermissionSetCallback& callback);
  ~DurableStoragePermissionInfoBarDelegate() override = default;

  base::string16 GetMessageText() const override;

  GURL requesting_frame_;
  std::string display_languages_;

  DISALLOW_COPY_AND_ASSIGN(DurableStoragePermissionInfoBarDelegate);
};

#endif  // CHROME_BROWSER_STORAGE_DURABLE_STORAGE_PERMISSION_INFOBAR_DELEGATE_H_
