# AVInput Plugin - Product Overview

## Product Description

The AVInput plugin is an enterprise-grade WPEFramework component that provides comprehensive audio/video input management capabilities for RDK-based devices. It enables applications to discover, monitor, and control external AV inputs including HDMI and Composite sources, making it essential for smart TVs, set-top boxes, and streaming devices that need to integrate external content sources.

## Key Features

### 1. Multi-Input Support
- **HDMI Input Management:** Support for multiple HDMI input ports with hot-plug detection
- **Composite Input Support:** Legacy analog video input handling (CVBS)
- **Dynamic Device Discovery:** Automatic enumeration of available input devices
- **Connection Status Monitoring:** Real-time detection of device connect/disconnect events

### 2. Advanced Video Capabilities
- **Signal Detection:** Automatic detection of input signal presence and quality
- **Video Mode Information:** Access to resolution, frame rate, and format details
- **Content Protection:** HDCP status and version information
- **HDR Support:** Detection and reporting of HDR10, HLG, and Dolby Vision formats

### 3. Gaming Features
- **ALLM (Auto Low Latency Mode):** Detection and status reporting for gaming content
- **VRR (Variable Refresh Rate):** Support for FreeSync and HDMI VRR technologies
- **Game Feature Detection:** Automatic identification of gaming-optimized content
- **Low Latency Monitoring:** Real-time status updates for latency-sensitive applications

### 4. Audio Management
- **Audio Format Detection:** Identification of audio codec and channel configuration
- **Volume Control:** Independent control of primary and input audio levels
- **Audio Mixing:** Support for audio mixing with system audio
- **SAP/MTS Support:** Secondary audio program handling for broadcast content

### 5. Content Intelligence
- **AVI InfoFrame Parsing:** Extraction of content type information from HDMI metadata
- **Video Stream Analysis:** Detailed analysis of video stream characteristics
- **Format Conversion Tracking:** Monitoring of upscaling/downscaling operations
- **Aspect Ratio Detection:** Automatic identification of content aspect ratios

## Use Cases

### Smart TV Integration
Enable seamless switching between streaming content and external devices:
- Display available HDMI inputs in source selection menu
- Show connection status and preview thumbnails
- Automatically detect gaming consoles and optimize display settings
- Handle HDCP-protected content appropriately

### Set-Top Box Applications
Integrate cable/satellite boxes with external AV equipment:
- Pass-through HDMI connections with monitoring
- Audio mixing for picture-in-picture scenarios
- Automatic input switching based on signal presence
- EDID management for optimal source compatibility

### Hospitality and Commercial Displays
Provide enterprise-grade input management:
- Centralized control of multiple input sources
- Remote monitoring of connection status
- Automated input routing based on schedules
- Integration with room control systems

### Gaming Consoles
Optimize display settings for gaming:
- Automatic detection of gaming mode requirements
- Enable VRR/ALLM when gaming console is detected
- Display latency information to users
- Dynamic refresh rate adjustment

## API Capabilities

### JSON-RPC Methods (org.rdk.AVInput.1)

**Device Management:**
- `numberOfInputs` - Query total number of available inputs
- `getInputDevices` - Enumerate all input devices with status
- `getSupportedGameFeatures` - Query gaming feature support

**Property Queries:**
- `currentVideoMode` - Get active video mode and resolution
- `contentProtected` - Check HDCP protection status
- `getAudioFormat` - Retrieve audio codec and channel info
- `getVideoContentType` - Get content type from AVI InfoFrame

**Configuration:**
- `setEdidVersion` - Configure EDID version for compatibility
- `setMixerLevels` - Adjust audio mixing levels
- `setVideoRectangle` - Configure video display area

**Status Information:**
- `getApiVersionNumber` - Query plugin version
- `getAVInputStatus` - Comprehensive input status report
- `getAllmStatus` - Get ALLM feature status
- `getSupportedVrrType` - Query VRR technology support

### Event Notifications

**Connection Events:**
- `onDevicesChanged` - Input device list changed
- `onSignalChanged` - Signal presence/quality changed
- `onStatusChanged` - Input status changed (active/inactive)

**Content Events:**
- `onVideoStreamInfoUpdate` - Video stream parameters changed
- `onGameFeatureStatusUpdate` - Gaming feature status changed
- `onAviContentTypeUpdate` - Content type metadata changed

## Integration Benefits

### Developer Experience
- **Simple JSON-RPC API:** Easy integration with web-based UIs
- **Comprehensive Events:** Real-time notifications for all state changes
- **Well-Documented:** Complete API reference and examples
- **Thunder Framework:** Leverages robust plugin infrastructure

### Performance Characteristics
- **Low Latency:** Sub-100ms event notification for hot-plug events
- **Efficient Polling:** Optimized HAL queries minimize CPU overhead
- **Asynchronous Operations:** Non-blocking API for responsive UIs
- **Resource Management:** Automatic cleanup and resource deallocation

### Reliability
- **Error Recovery:** Graceful handling of HAL failures
- **State Consistency:** Reliable state tracking across input changes
- **Thread Safety:** Fully thread-safe API implementation
- **Production Tested:** Deployed across millions of RDK devices

## Platform Support

### Hardware Requirements
- Device Settings (DS) HAL implementation
- HDMI input hardware (1-4 ports typical)
- Optional: Composite input hardware

### Software Requirements
- WPEFramework (Thunder) R4.4+
- RDK Device Settings libraries
- IARMBus (for inter-process communication)

### Supported Platforms
- RDK-V (Video) devices
- Broadcom, Amlogic, Realtek SoCs
- ARM and x86 architectures

## Configuration and Deployment

### Build Options
- Configurable input type support (HDMI, Composite)
- Telemetry integration
- Debug logging levels
- Feature enablement flags

### Runtime Configuration
- JSON configuration file support
- Dynamic EDID management
- Custom audio mixing profiles
- Platform-specific optimizations

## Versioning

**Current Version:** 1.7.1
- Major version changes indicate API breaking changes
- Minor version changes add new features
- Patch versions for bug fixes and improvements

## Security and Compliance

- **Content Protection:** Full HDCP 1.4/2.2 support
- **Access Control:** Integrated with Thunder security framework
- **Privacy:** No collection of user viewing data
- **Compliance:** Meets HDMI Adopter requirements

## Future Roadmap

- **HDMI 2.1 Features:** Enhanced gaming mode support, eARC
- **8K Support:** Ultra-high resolution input handling
- **AI Integration:** Intelligent content type detection
- **Cloud Services:** Remote input monitoring and control

---

*The AVInput plugin represents a mature, production-ready solution for AV input management in modern RDK devices, combining powerful functionality with ease of integration and rock-solid reliability.*
