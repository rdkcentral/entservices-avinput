/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

#pragma once
#include <interfaces/json/JAVInput.h>
#include <core/JSON.h>
#include <interfaces/IAVInput.h>

// The following is extracted from JsonData_AVInput.h, as Thunder 4.x doesn't support iterators in events.
// Because of this AVInput handles the event and converts the iterator to a JSON array string. When we move to 5.x we can
// remove this class and use the iterator directly in the event.

namespace WPEFramework
{
    namespace Plugin
    {
        class InputDeviceJson : public Core::JSON::Container {
            public:
                InputDeviceJson()
                    : Core::JSON::Container()
                {
                    _Init();
                }

                InputDeviceJson(const InputDeviceJson& _other)
                    : Core::JSON::Container()
                    , Id(_other.Id)
                    , Locator(_other.Locator)
                    , Connected(_other.Connected)
                {
                    _Init();
                }

                InputDeviceJson& operator=(const InputDeviceJson& _rhs)
                {
                    Id = _rhs.Id;
                    Locator = _rhs.Locator;
                    Connected = _rhs.Connected;
                    return (*this);
                }

                InputDeviceJson(const Exchange::IAVInput::InputDevice& _other)
                    : Core::JSON::Container()
                {
                    Id = _other.id;
                    Locator = _other.locator;
                    Connected = _other.connected;
                    _Init();
                }

                InputDeviceJson& operator=(const Exchange::IAVInput::InputDevice& _rhs)
                {
                    Id = _rhs.id;
                    Locator = _rhs.locator;
                    Connected = _rhs.connected;
                    return (*this);
                }

                operator Exchange::IAVInput::InputDevice() const
                {
                    Exchange::IAVInput::InputDevice _value{};
                    _value.id = Id;
                    _value.locator = Locator;
                    _value.connected = Connected;
                    return (_value);
                }

                bool IsValid() const
                {
                    return (true);
                }

            private:
                void _Init()
                {
                    Add(_T("id"), &Id);
                    Add(_T("locator"), &Locator);
                    Add(_T("connected"), &Connected);
                }

            public:
                Core::JSON::DecSInt32 Id; // id
                Core::JSON::String Locator; // locator
                Core::JSON::Boolean Connected; // connected

        }; // class InputDeviceJson
    } // namespace Plugin
} // namespace WPEFramework
