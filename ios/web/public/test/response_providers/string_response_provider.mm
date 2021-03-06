// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios/web/public/test/response_providers/string_response_provider.h"

namespace web {

StringResponseProvider::StringResponseProvider(const std::string& response_body)
    : response_body_(response_body) {}

bool StringResponseProvider::CanHandleRequest(const Request& request) {
  return true;
}

void StringResponseProvider::GetResponseHeadersAndBody(
    const Request& request,
    scoped_refptr<net::HttpResponseHeaders>* headers,
    std::string* response_body) {
  *headers = GetDefaultResponseHeaders();
  *response_body = response_body_;
}

}  // namespace web
