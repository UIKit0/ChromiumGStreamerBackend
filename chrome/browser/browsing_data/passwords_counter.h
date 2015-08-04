// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_BROWSING_DATA_PASSWORDS_COUNTER_H_
#define CHROME_BROWSER_BROWSING_DATA_PASSWORDS_COUNTER_H_

#include "chrome/browser/browsing_data/browsing_data_counter.h"

class PasswordsCounter: public BrowsingDataCounter {
 public:
  PasswordsCounter();
  ~PasswordsCounter() override;

  const std::string& GetPrefName() const override;

 private:
  const std::string pref_name_;

  void Count() override;
};

#endif  // CHROME_BROWSER_BROWSING_DATA_PASSWORDS_COUNTER_H_
