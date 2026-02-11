# AVInput Plugin Architecture

## Overview

The AVInput plugin is a WPEFramework (Thunder) plugin that provides comprehensive AV input management functionality for RDK devices. It serves as the primary interface for handling HDMI and Composite video inputs, exposing device information, signal characteristics, and content metadata to client applications.

## System Architecture

### High-Level Component Structure

```
┌─────────────────────────────────────────────────────────────┐
│              Client Applications (JSON-RPC)                  │
└──────────────────┬──────────────────────────────────────────┘
                   │
┌──────────────────▼──────────────────────────────────────────┐
│              WPEFramework Core                               │
│  ┌───────────────────────────────────────────────────┐      │
│  │         AVInput Plugin (PluginHost)               │      │
│  │  ┌─────────────────────────────────────────┐     │      │
│  │  │      AVInput (JSONRPC Interface)        │     │      │
│  │  └──────────────┬──────────────────────────┘     │      │
│  │                 │                                 │      │
│  │  ┌──────────────▼──────────────────────────┐     │      │
│  │  │    AVInputImplementation (Core Logic)   │     │      │
│  │  │  - Device Management                    │     │      │
│  │  │  - Signal Processing                    │     │      │
│  │  │  - Event Notification                   │     │      │
│  │  └──────────────┬──────────────────────────┘     │      │
│  └─────────────────┼──────────────────────────────────┘    │
└────────────────────┼─────────────────────────────────────────┘
                     │
┌────────────────────▼─────────────────────────────────────────┐
│           Device Settings (DS) HAL Layer                      │
│  ┌──────────────────────┐  ┌─────────────────────────┐      │
│  │  HdmiInput           │  │  CompositeInput         │      │
│  │  - Port Management   │  │  - Port Management      │      │
│  │  - Signal Detection  │  │  - Signal Detection     │      │
│  │  - EDID Handling     │  │  - Format Detection     │      │
│  └──────────────────────┘  └─────────────────────────┘      │
└──────────────────────────────────────────────────────────────┘
```

## Core Components

### 1. AVInput (JSONRPC Layer)

**Responsibilities:**
- Exposes JSON-RPC API interface to client applications
- Manages plugin lifecycle (initialization, deinitialization)
- Registers/unregisters notification callbacks
- Handles communication between WPEFramework and implementation layer

**Key Interfaces:**
- `PluginHost::IPlugin` - Plugin lifecycle management
- `PluginHost::JSONRPC` - JSON-RPC method handling
- `Exchange::IAVInput` - AV input interface aggregation

**Design Pattern:**
- Uses aggregation pattern to expose IAVInput interface
- Implements notification delegation to forward events to registered clients

### 2. AVInputImplementation (Business Logic Layer)

**Responsibilities:**
- Core business logic for AV input operations
- Device Settings HAL interaction and abstraction
- Event handling and notification dispatch
- State management for input devices

**Key Features:**
- Multi-input support (HDMI, Composite)
- Real-time signal and status monitoring
- Video stream information management
- Game feature detection (ALLM, VRR)
- Content type identification (AVI InfoFrame)

**Event Processing:**
Uses a Job dispatch mechanism for asynchronous event handling:
- Events are queued and processed on the thread pool
- Supports multiple concurrent notification listeners
- Type-safe event parameter handling using boost::variant

### 3. AVInputUtils (Helper Layer)

**Responsibilities:**
- Common utility functions for input type conversion
- String manipulation and validation
- Type mapping between API and HAL layers

**Key Functions:**
- Input type conversion (string ↔ enum)
- Locator URL parsing and generation
- Device ID validation

## Data Flow

### Device Enumeration Flow
```
Client Request → AVInput::getInputDevices()
    → AVInputImplementation::GetInputDevices()
    → DS HAL (HdmiInput/CompositeInput).getNumberOfInputs()
    → Iterate ports, check connection status
    → Build device list with locators
    → Return JSON response to client
```

### Signal Change Event Flow
```
Hardware Signal Change
    → DS HAL Event Callback
    → AVInputImplementation::onSignalChanged()
    → Create Job with event parameters
    → Dispatch Job to thread pool
    → Job::Dispatch() iterates notification list
    → Fire OnSignalChanged() for each registered client
    → Client receives event notification
```

## Threading Model

- **Main Thread:** Plugin initialization, configuration, and JSON-RPC request handling
- **Event Thread Pool:** Asynchronous event dispatch through Job mechanism
- **HAL Thread:** Device Settings callbacks (converted to Jobs for dispatch)

**Synchronization:**
- `_adminLock` (Critical Section) protects notification lists
- Lock acquired only during registration/unregistration operations
- Event dispatch operates without locks (uses snapshots)

## Dependencies

### External Libraries
- **WPEFramework Core:** Plugin infrastructure, JSON-RPC, threading
- **Device Settings (DS):** HAL abstraction for AV devices
- **IARMBus:** Inter-process communication (optional)
- **Boost:** Type-safe variant for event parameters

### Internal Dependencies
- **UtilsJsonRpc.h:** JSON-RPC helper macros and utilities
- **UtilsLogging.h:** Logging infrastructure

## API Interface

The plugin implements the `Exchange::IAVInput` COM interface with the following method categories:

1. **Device Management:** GetInputDevices, GetSupportedGameFeatures
2. **Property Access:** GetCurrentVideoMode, GetAudioFormat, GetVideoContentType
3. **Control Operations:** SetEdidVersion, SetMixerLevels
4. **Notification Management:** Register/Unregister for 6 event types

## Configuration

Plugin configuration via JSON:
- Input device mappings
- Default volume levels
- EDID version settings
- Telemetry options

Configuration file: `AVInput.config`

## Error Handling

- Uses Thunder error codes (Core::ERROR_*)
- Device Settings exceptions caught and logged
- Graceful degradation on HAL failures
- Comprehensive error reporting via return codes

## Performance Considerations

- Lightweight event notification (async dispatch)
- Minimal lock contention (registration-only locking)
- Efficient device iteration (cached port counts)
- Zero-copy event parameter passing where possible

## Security

- No direct hardware access (abstracted via DS HAL)
- Input validation on all API parameters
- Secure handling of HDCP status information
- No authentication required (delegated to WPEFramework)

---

*This architecture supports extensibility for future input types and maintains clean separation between API surface, business logic, and hardware abstraction layers.*
