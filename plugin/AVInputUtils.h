/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2025 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.Fv
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <string>

static const std::string    INPUT_TYPE_STRING_ALL       = "ALL";
static const std::string    INPUT_TYPE_STRING_HDMI      = "HDMI";
static const std::string    INPUT_TYPE_STRING_COMPOSITE = "COMPOSITE";

static const int            INPUT_TYPE_INT_ALL          = -1;
static const int            INPUT_TYPE_INT_HDMI         = 0;
static const int            INPUT_TYPE_INT_COMPOSITE    = 1;

namespace WPEFramework {
    namespace Plugin {
        class AVInputUtils {
        public:
            static const int getTypeOfInput(const std::string& type);
            static const std::string& getTypeOfInput(const int type);

        private:
            AVInputUtils() = delete;
        };

    } // namespace WPEFramework
} // namespace Plugin
