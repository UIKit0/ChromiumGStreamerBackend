// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stddef.h>
#include <stdint.h>

#include "base/macros.h"
#include "chrome/browser/media_galleries/linux/mtp_device_object_enumerator.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

struct MtpFileEntryData {
  const char* const name;
  int64_t size;
  bool is_directory;
  time_t modification_time;
};

const MtpFileEntryData kTestCases[] = {
  { "Foo", 123, false, 321 },
  { "Bar", 234, true, 432 },
  { "Baaz", 345, false, 543 },
};

void TestEnumeratorIsEmpty(MTPDeviceObjectEnumerator* enumerator) {
  EXPECT_EQ(0, enumerator->Size());
  EXPECT_FALSE(enumerator->IsDirectory());
  EXPECT_TRUE(enumerator->LastModifiedTime().is_null());
}

void TestNextEntryIsEmpty(MTPDeviceObjectEnumerator* enumerator) {
  EXPECT_TRUE(enumerator->Next().empty());
}

typedef testing::Test MTPDeviceObjectEnumeratorTest;

TEST_F(MTPDeviceObjectEnumeratorTest, Empty) {
  std::vector<MtpFileEntry> entries;
  MTPDeviceObjectEnumerator enumerator(entries);
  TestEnumeratorIsEmpty(&enumerator);
  TestNextEntryIsEmpty(&enumerator);
  TestNextEntryIsEmpty(&enumerator);
  TestEnumeratorIsEmpty(&enumerator);
}

TEST_F(MTPDeviceObjectEnumeratorTest, Traversal) {
  std::vector<MtpFileEntry> entries;
  for (size_t i = 0; i < arraysize(kTestCases); ++i) {
    MtpFileEntry entry;
    entry.set_file_name(kTestCases[i].name);
    entry.set_file_size(kTestCases[i].size);
    entry.set_file_type(kTestCases[i].is_directory ?
        MtpFileEntry::FILE_TYPE_FOLDER :
        MtpFileEntry::FILE_TYPE_OTHER);
    entry.set_modification_time(kTestCases[i].modification_time);
    entries.push_back(entry);
  }
  MTPDeviceObjectEnumerator enumerator(entries);
  TestEnumeratorIsEmpty(&enumerator);
  TestEnumeratorIsEmpty(&enumerator);
  for (size_t i = 0; i < arraysize(kTestCases); ++i) {
    EXPECT_EQ(kTestCases[i].name, enumerator.Next().value());
    EXPECT_EQ(kTestCases[i].size, enumerator.Size());
    EXPECT_EQ(kTestCases[i].is_directory, enumerator.IsDirectory());
    EXPECT_EQ(kTestCases[i].modification_time,
              enumerator.LastModifiedTime().ToTimeT());
  }
  TestNextEntryIsEmpty(&enumerator);
  TestNextEntryIsEmpty(&enumerator);
  TestEnumeratorIsEmpty(&enumerator);
  TestNextEntryIsEmpty(&enumerator);
  TestEnumeratorIsEmpty(&enumerator);
}

}  // namespace
