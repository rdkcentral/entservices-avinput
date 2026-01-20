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

#include "AVInputUtils.h"
#include <stdexcept>

namespace WPEFramework {
namespace Plugin {

const int AVInputUtils::getTypeOfInput(const std::string& sType) {
    if (sType == INPUT_TYPE_STRING_HDMI) {
        return INPUT_TYPE_INT_HDMI;
    }
    else if (sType == INPUT_TYPE_STRING_COMPOSITE) {
        return INPUT_TYPE_INT_COMPOSITE;
    }
    else if (sType == INPUT_TYPE_STRING_ALL) {
        return INPUT_TYPE_INT_ALL;
    }
    else {
        throw std::invalid_argument("Invalid type of INPUT, please specify HDMI/COMPOSITE/ALL");
    }
}

const std::string& AVInputUtils::getTypeOfInput(const int type) {
    switch(type) {
        case INPUT_TYPE_INT_HDMI: return INPUT_TYPE_STRING_HDMI;
        case INPUT_TYPE_INT_COMPOSITE: return INPUT_TYPE_STRING_COMPOSITE;
        case INPUT_TYPE_INT_ALL: return INPUT_TYPE_STRING_ALL;
        default: throw std::invalid_argument("Invalid input type");
    }
}

} // namespace WPEFramework
} // namespace Plugin
