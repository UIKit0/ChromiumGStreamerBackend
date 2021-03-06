// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/precache/core/precache_url_table.h"

#include <string>

#include "base/logging.h"
#include "sql/connection.h"
#include "sql/statement.h"

using sql::Statement;

namespace {

// Returns the spec of the given URL.
std::string GetKey(const GURL& url) {
  return url.spec();
}

}  // namespace

namespace precache {

PrecacheURLTable::PrecacheURLTable() : db_(NULL) {}

PrecacheURLTable::~PrecacheURLTable() {}

bool PrecacheURLTable::Init(sql::Connection* db) {
  DCHECK(!db_);  // Init must only be called once.
  DCHECK(db);    // The database connection must be non-NULL.
  db_ = db;
  return CreateTableIfNonExistent();
}

void PrecacheURLTable::AddURL(const GURL& url,
                              int64_t referrer_host_id,
                              bool is_precached,
                              const base::Time& precache_time) {
  Statement statement(
      db_->GetCachedStatement(SQL_FROM_HERE,
                              "INSERT OR REPLACE INTO precache_urls (url, "
                              "referrer_host_id, was_used, is_precached, time) "
                              "VALUES(?,?,0,?,?)"));

  statement.BindString(0, GetKey(url));
  statement.BindInt64(1, referrer_host_id);
  statement.BindInt64(2, is_precached ? 1 : 0);
  statement.BindInt64(3, precache_time.ToInternalValue());
  statement.Run();
}

bool PrecacheURLTable::IsURLPrecached(const GURL& url) {
  Statement statement(db_->GetCachedStatement(
      SQL_FROM_HERE,
      "SELECT time FROM precache_urls WHERE url=? and is_precached=1"));

  statement.BindString(0, GetKey(url));
  return statement.Step();
}

bool PrecacheURLTable::IsURLPrecachedAndUnused(const GURL& url) {
  Statement statement(db_->GetCachedStatement(SQL_FROM_HERE,
                                              "SELECT time FROM precache_urls "
                                              "WHERE url=? and is_precached=1 "
                                              "and was_used=0"));

  statement.BindString(0, GetKey(url));
  return statement.Step();
}

void PrecacheURLTable::SetPrecachedURLAsUsed(const GURL& url) {
  Statement statement(
      db_->GetCachedStatement(SQL_FROM_HERE,
                              "UPDATE precache_urls SET was_used=1, "
                              "is_precached=0 "
                              "WHERE url=? and was_used=0 and is_precached=1"));

  statement.BindString(0, GetKey(url));
  statement.Run();
}

void PrecacheURLTable::SetURLAsNotPrecached(const GURL& url) {
  Statement statement(
      db_->GetCachedStatement(SQL_FROM_HERE,
                              "UPDATE precache_urls SET is_precached=0 "
                              "WHERE url=? and is_precached=1"));
  statement.BindString(0, GetKey(url));
  statement.Run();
}

void PrecacheURLTable::GetURLListForReferrerHost(
    int64_t referrer_host_id,
    std::vector<GURL>* used_urls,
    std::vector<GURL>* unused_urls) {
  Statement statement(db_->GetCachedStatement(
      SQL_FROM_HERE,
      "SELECT url, was_used from precache_urls where referrer_host_id=?"));
  statement.BindInt64(0, referrer_host_id);
  while (statement.Step()) {
    GURL url(statement.ColumnString(0));
    if (statement.ColumnInt(1))
      used_urls->push_back(url);
    else
      unused_urls->push_back(url);
  }
}

void PrecacheURLTable::ClearAllForReferrerHost(int64_t referrer_host_id) {
  Statement statement(db_->GetCachedStatement(
      SQL_FROM_HERE,
      "UPDATE precache_urls SET was_used=0 WHERE referrer_host_id=?"));

  statement.BindInt64(0, referrer_host_id);
  statement.Run();
}

void PrecacheURLTable::DeleteAllPrecachedBefore(const base::Time& delete_end) {
  Statement statement(db_->GetCachedStatement(
      SQL_FROM_HERE, "DELETE FROM precache_urls WHERE time < ?"));

  statement.BindInt64(0, delete_end.ToInternalValue());
  statement.Run();
}

void PrecacheURLTable::DeleteAll() {
  Statement statement(
      db_->GetCachedStatement(SQL_FROM_HERE, "DELETE FROM precache_urls"));

  statement.Run();
}

void PrecacheURLTable::GetAllDataForTesting(std::map<GURL, base::Time>* map) {
  map->clear();

  Statement statement(db_->GetCachedStatement(
      SQL_FROM_HERE,
      "SELECT url, time FROM precache_urls where is_precached=1"));

  while (statement.Step()) {
    GURL url = GURL(statement.ColumnString(0));
    (*map)[url] = base::Time::FromInternalValue(statement.ColumnInt64(1));
  }
}

bool PrecacheURLTable::CreateTableIfNonExistent() {
  if (!db_->DoesTableExist("precache_urls")) {
    return db_->Execute(
        "CREATE TABLE precache_urls "
        "(url TEXT PRIMARY KEY, referrer_host_id INTEGER, was_used INTEGER, "
        "is_precached INTEGER, "
        "time INTEGER)");
  } else {
    // Migrate the table by creating the missing columns.
    if (!db_->DoesColumnExist("precache_urls", "was_used") &&
        !db_->Execute("ALTER TABLE precache_urls ADD COLUMN was_used INTEGER"))
      return false;
    if (!db_->DoesColumnExist("precache_urls", "is_precached") &&
        !db_->Execute(
            "ALTER TABLE precache_urls ADD COLUMN is_precached INTEGER"))
      return false;
    if (!db_->DoesColumnExist("precache_urls", "referrer_host_id") &&
        !db_->Execute(
            "ALTER TABLE precache_urls ADD COLUMN referrer_host_id INTEGER"))
      return false;
  }
  return true;
}

}  // namespace precache
