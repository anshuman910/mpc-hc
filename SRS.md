# Software Requirements Specification (SRS)
## Media Player Classic - Home Cinema (MPC-HC)
### Version 2.0 - Optimized for Low-End Hardware

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [System Overview](#2-system-overview)
3. [Architecture](#3-architecture)
4. [Core Components](#4-core-components)
5. [End-to-End Workflows](#5-end-to-end-workflows)
6. [Performance Optimizations](#6-performance-optimizations)
7. [Data Flow](#7-data-flow)
8. [Configuration Management](#8-configuration-management)
9. [Error Handling](#9-error-handling)
10. [Hardware Detection & Optimization](#10-hardware-detection--optimization)

---

## 1. Introduction

### 1.1 Purpose
This document provides a comprehensive Software Requirements Specification for MPC-HC (Media Player Classic - Home Cinema), a DirectShow-based media player for Windows. This SRS documents the complete end-to-end logic, architecture, and workflows of the application, with special focus on optimizations for low-end hardware systems.

### 1.2 Scope
MPC-HC is a free and open-source video and audio player that supports a wide range of media formats through DirectShow filters and internal codecs (LAV Filters). The application provides a simple, effective user interface with extensive customization options.

### 1.3 Definitions and Acronyms
- **DirectShow**: Microsoft's media framework for Windows
- **LAV Filters**: Internal codec filters (LAV Splitter, LAV Video, LAV Audio)
- **EVR**: Enhanced Video Renderer
- **VMR9**: Video Mixing Renderer 9
- **MPCVR**: MPC Video Renderer
- **madVR**: High-quality video renderer (external)
- **SRS**: Software Requirements Specification

---

## 2. System Overview

### 2.1 System Architecture

MPC-HC follows a modular architecture based on DirectShow filter graph:

```
┌─────────────────────────────────────────────────────────┐
│                    MPC-HC Application                    │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │
│  │  Main Frame  │  │  Graph Thread │  │  UI Thread   │  │
│  │  (MainFrm)   │  │ (GraphThread)│  │  (Main UI)   │  │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘  │
│         │                 │                  │          │
│         └─────────────────┼──────────────────┘          │
│                           │                             │
│         ┌─────────────────▼─────────────────┐          │
│         │      DirectShow Filter Graph       │          │
│         │  ┌────────┐  ┌────────┐  ┌─────┐│          │
│         │  │ Source │→ │Decoder │→ │Render││          │
│         │  │ Filter │  │ Filter │  │ Filter││          │
│         │  └────────┘  └────────┘  └─────┘│          │
│         └───────────────────────────────────┘          │
└─────────────────────────────────────────────────────────┘
```

### 2.2 Key Technologies
- **Framework**: MFC (Microsoft Foundation Classes)
- **Media Framework**: DirectShow
- **Rendering**: Direct3D 9, EVR, VMR9, MPCVR
- **Audio**: DirectSound, WASAPI, SaneAR
- **Subtitles**: Internal renderer, libass, VSFilter
- **Codecs**: LAV Filters (FFmpeg-based)

---

## 3. Architecture

### 3.1 Application Structure

#### 3.1.1 Main Application Class (`CMPlayerCApp`)
- **Location**: `src/mpc-hc/mplayerc.h/cpp`
- **Purpose**: Application entry point, settings management, profile I/O
- **Key Responsibilities**:
  - Application initialization (`InitInstance`)
  - Settings persistence (Registry/INI)
  - Single instance management
  - Command-line processing
  - Event routing

#### 3.1.2 Main Frame (`CMainFrame`)
- **Location**: `src/mpc-hc/MainFrm.h/cpp`
- **Purpose**: Main window, playback control, filter graph management
- **Key Responsibilities**:
  - Window management and UI
  - Media playback control (Play/Pause/Stop/Seek)
  - Filter graph building and management
  - Subtitle handling
  - Playlist management
  - Fullscreen mode
  - Keyboard shortcuts and mouse controls

#### 3.1.3 Graph Thread (`CGraphThread`)
- **Location**: `src/mpc-hc/GraphThread.h/cpp`
- **Purpose**: Separate thread for filter graph operations
- **Key Responsibilities**:
  - Asynchronous graph building
  - Media file opening
  - Graph reset operations
  - Display change handling

#### 3.1.4 Base Graph (`CBaseGraph`)
- **Location**: `src/mpc-hc/BaseGraph.h/cpp`
- **Purpose**: DirectShow filter graph wrapper
- **Key Responsibilities**:
  - Filter graph construction
  - Media control (IMediaControl)
  - Media seeking (IMediaSeeking)
  - Event handling (IMediaEventEx)
  - Video/audio interfaces

### 3.2 Filter Architecture

#### 3.2.1 Source Filters
- **BaseSource**: Base class for source filters
- **AsyncReader**: Asynchronous file reading
- **CDDAReader**: Audio CD reading
- **VTSReader**: DVD VTS reading

#### 3.2.2 Parser/Splitter Filters
- **BaseSplitter**: Base splitter implementation
- **DSMSplitter**: DSM format splitter
- **LAV Splitter**: Internal LAV splitter (external project)

#### 3.2.3 Transform Filters
- **LAV Video**: Video decoder (H.264, HEVC, AV1, etc.)
- **LAV Audio**: Audio decoder (AAC, AC3, DTS, etc.)
- **AudioSwitcher**: Audio format conversion
- **BufferFilter**: Buffering filter
- **DeCSSFilter**: DVD decryption

#### 3.2.4 Renderer Filters
- **VideoRenderers**: EVR, VMR9, MPCVR, madVR support
- **SyncClock**: Synchronization clock
- **Audio Renderers**: DirectSound, WASAPI, SaneAR, MPC Audio Renderer

---

## 4. Core Components

### 4.1 Settings Management (`CAppSettings`)

#### 4.1.1 Settings Storage
- **Registry**: `HKEY_CURRENT_USER\Software\MPC-HC\MPC-HC`
- **INI File**: `mpc-hc.ini` (optional, user-configurable)
- **Profile System**: Lazy-loaded, cached profile access

#### 4.1.2 Key Settings Categories
1. **Player Settings**
   - Window position/size
   - Fullscreen behavior
   - Playback options
   - Volume/balance

2. **Playback Settings**
   - Speed control
   - Loop mode
   - After playback action
   - Fast seek method

3. **Output Settings**
   - Video renderer selection
   - Audio renderer selection
   - Subtitle renderer

4. **Subtitle Settings**
   - Font, size, colors
   - Position, delay
   - Language preferences
   - Auto-download settings

5. **Performance Settings**
   - Stream position poller interval
   - Priority class
   - Hardware acceleration

### 4.2 Filter Graph Management

#### 4.2.1 Graph Building Process

**Step 1: Source Filter Selection**
```
1. Check file extension/URL
2. Determine media type
3. Select appropriate source filter:
   - LAV Splitter (for most formats)
   - Internal splitter (for specific formats)
   - External splitter (user-configured)
```

**Step 2: Decoder Selection**
```
1. Query source filter for output pins
2. For each pin:
   - Check media type (video/audio/subtitle)
   - Select decoder:
     * LAV Video (for video)
     * LAV Audio (for audio)
     * Internal decoder (if enabled)
     * External decoder (user-configured)
```

**Step 3: Renderer Selection**
```
1. Video renderer:
   - Check user preference (EVR/VMR9/MPCVR/madVR)
   - Create renderer filter
   - Connect video decoder output
   
2. Audio renderer:
   - Check user preference (DirectSound/WASAPI/SaneAR/MPC)
   - Create audio renderer
   - Connect audio decoder output
```

**Step 4: Subtitle Integration**
```
1. Load external subtitle files
2. Connect subtitle source filter
3. Connect subtitle renderer to video renderer
```

#### 4.2.2 Graph State Management
- **Stopped**: Graph built but not running
- **Paused**: Graph paused, ready to play
- **Running**: Graph actively playing media

### 4.3 Playback Control

#### 4.3.1 Playback States
```
STOPPED → PAUSED → RUNNING
   ↑         ↓         ↓
   └─────────┴─────────┘
```

#### 4.3.2 Seek Operation
1. **User initiates seek** (mouse click on seekbar, keyboard shortcut)
2. **Calculate target position** (from seekbar position or time)
3. **Check fast seek mode**:
   - If enabled: Seek to nearest keyframe
   - If disabled: Seek to exact position
4. **Call IMediaSeeking::SetPositions**
5. **Update UI** (seekbar, time display)

#### 4.3.3 Speed Control
1. **User changes playback rate** (keyboard shortcut, menu)
2. **Call IMediaSeeking::SetRate**
3. **Update audio renderer** (if supports pitch correction)
4. **Update UI** (playback rate display)

### 4.4 Subtitle System

#### 4.4.1 Subtitle Loading
1. **Internal Subtitles** (embedded in media):
   - Extracted by splitter
   - Passed to subtitle renderer

2. **External Subtitles**:
   - Auto-load: Search for subtitle files matching media name
   - Manual load: User selects file
   - Formats: SRT, ASS, SSA, VTT, PGS, etc.

#### 4.4.2 Subtitle Rendering
1. **Subtitle Renderer Selection**:
   - Internal renderer (default)
   - VSFilter (external, if available)
   - libass (for ASS/SSA, if enabled)

2. **Rendering Process**:
   - Parse subtitle file
   - Time-sync with video
   - Render text/graphics on video frame
   - Apply styling (font, colors, position)

### 4.5 Playlist Management

#### 4.5.1 Playlist Structure
- **PlaylistBar**: UI component for playlist
- **Playlist**: Data structure holding media files
- **Current Item**: Currently playing file

#### 4.5.2 Playlist Operations
1. **Add Files**:
   - Drag & drop
   - File menu → Open File
   - Command-line arguments

2. **Navigation**:
   - Next/Previous file
   - Random play
   - Loop playlist

3. **Playlist Persistence**:
   - Save to file (M3U, M3U8, CUE)
   - Load from file

---

## 5. End-to-End Workflows

### 5.1 Application Startup

```
1. CMPlayerCApp::InitInstance()
   ├─ Parse command-line arguments
   ├─ Check for existing instance (single instance mode)
   ├─ Initialize settings (CAppSettings::LoadSettings)
   ├─ Initialize hardware detection (CHardwareDetection::Initialize)
   ├─ Apply hardware-optimized settings
   ├─ Create main window (CMainFrame)
   ├─ Initialize DirectShow
   ├─ Register hotkeys
   └─ Start message loop
```

### 5.2 Opening a Media File

```
1. User Action (File → Open, Drag & Drop, Command-line)
   │
2. CMainFrame::OpenMedia()
   ├─ Create OpenMediaData structure
   ├─ Post message to GraphThread
   │
3. CGraphThread::OnOpen()
   ├─ Close existing graph (if any)
   ├─ Create new filter graph (CBaseGraph)
   ├─ Select source filter
   │   ├─ Check internal filters
   │   ├─ Check external filters
   │   └─ Use LAV Splitter (default)
   │
4. Build Filter Graph
   ├─ Add source filter to graph
   ├─ Enumerate output pins
   ├─ For each pin:
   │   ├─ Determine media type
   │   ├─ Select decoder
   │   │   ├─ Video: LAV Video
   │   │   ├─ Audio: LAV Audio
   │   │   └─ Subtitle: Subtitle source
   │   ├─ Add decoder to graph
   │   ├─ Connect source → decoder
   │   ├─ Select renderer
   │   │   ├─ Video: EVR/VMR9/MPCVR
   │   │   └─ Audio: DirectSound/WASAPI
   │   ├─ Add renderer to graph
   │   └─ Connect decoder → renderer
   │
5. Configure Renderers
   ├─ Set video window (IVideoWindow)
   ├─ Configure audio renderer
   ├─ Load external subtitles (if any)
   └─ Connect subtitle renderer
   │
6. Query Interfaces
   ├─ IMediaControl (playback control)
   ├─ IMediaSeeking (seeking)
   ├─ IBasicVideo (video info)
   ├─ IBasicAudio (audio control)
   └─ IMediaEventEx (events)
   │
7. Update UI
   ├─ Update window title
   ├─ Update seekbar range
   ├─ Update status bar
   ├─ Show video window
   └─ Post WM_POSTOPEN message
   │
8. CMainFrame::OnPostOpen()
   ├─ Start playback (if auto-play enabled)
   ├─ Update menus
   ├─ Update toolbars
   └─ Set timers (position polling, stats)
```

### 5.3 Playback Process

```
1. User clicks Play (or auto-play)
   │
2. IMediaControl::Run()
   ├─ Filter graph starts running
   ├─ Source filter reads data
   ├─ Decoders decode frames
   └─ Renderers display output
   │
3. Position Polling (Timer: TIMER_STREAMPOSPOLLER)
   ├─ Query current position (IMediaSeeking::GetCurrentPosition)
   ├─ Update seekbar position
   ├─ Update time display
   └─ Check for end of file
   │
4. Event Handling (WM_GRAPHNOTIFY)
   ├─ Get event from graph (IMediaEventEx::GetEvent)
   ├─ Handle events:
   │   ├─ EC_COMPLETE: End of file
   │   ├─ EC_ERRORABORT: Playback error
   │   ├─ EC_REPAINT: Repaint needed
   │   └─ EC_USER: Custom events
   │
5. End of File Handling
   ├─ Check after-playback setting
   ├─ Actions:
   │   ├─ DO_NOTHING: Stop
   │   ├─ PLAY_NEXT: Play next file
   │   ├─ REWIND: Restart current file
   │   ├─ CLOSE: Close player
   │   └─ EXIT: Exit application
```

### 5.4 Seeking Operation

```
1. User Action (seekbar click, keyboard shortcut)
   │
2. CMainFrame::SeekTo()
   ├─ Calculate target position (REFERENCE_TIME)
   ├─ Check fast seek mode
   │   ├─ FASTSEEK_NEAREST_KEYFRAME: Seek to keyframe
   │   └─ FASTSEEK_EXACT: Seek to exact position
   ├─ Call IMediaSeeking::SetPositions()
   │   ├─ dwCurrentFlags = AM_SEEKING_AbsolutePositioning
   │   └─ pCurrent = target position
   │
3. Graph Processing
   ├─ Source filter seeks to position
   ├─ Decoders flush buffers
   ├─ Decoders seek to keyframe (if fast seek)
   └─ Renderers update display
   │
4. UI Update
   ├─ Update seekbar position
   ├─ Update time display
   └─ Update OSD (if enabled)
```

### 5.5 Subtitle Loading

```
1. Auto-Load Subtitles (on file open)
   ├─ Extract base filename (without extension)
   ├─ Search for subtitle files:
   │   ├─ Same directory as media file
   │   ├─ Subtitle extensions: .srt, .ass, .ssa, .vtt, .sub
   │   └─ Language codes: .en.srt, .eng.srt, etc.
   ├─ Match by filename
   └─ Load first match (or best language match)
   │
2. Manual Load (File → Subtitles → Load Subtitles)
   ├─ Show file dialog
   ├─ User selects subtitle file
   └─ Load selected file
   │
3. Subtitle Processing
   ├─ Parse subtitle file format
   ├─ Extract subtitle entries (time, text)
   ├─ Create subtitle stream (ISubStream)
   └─ Connect to subtitle renderer
   │
4. Subtitle Rendering
   ├─ Subtitle renderer receives time updates
   ├─ Find active subtitle entries (current time)
   ├─ Render text on video frame
   └─ Apply styling (font, colors, position)
```

### 5.6 Fullscreen Mode

```
1. User Action (F11, double-click, menu)
   │
2. CMainFrame::OnFullscreen()
   ├─ Check current state
   │   ├─ If windowed → Enter fullscreen
   │   └─ If fullscreen → Exit fullscreen
   │
3. Enter Fullscreen
   ├─ Hide window decorations
   ├─ Resize window to screen size
   ├─ Move window to primary monitor (or selected monitor)
   ├─ Set window style (WS_POPUP, WS_EX_TOPMOST)
   ├─ Hide controls (if auto-hide enabled)
   ├─ Update video window size
   └─ Set fullscreen flag
   │
4. Exit Fullscreen
   ├─ Restore window style
   ├─ Restore window size/position
   ├─ Show controls
   └─ Update video window size
```

### 5.7 Settings Persistence

```
1. Settings Change (user modifies option)
   │
2. Update CAppSettings member variable
   │
3. Settings Save (on application close, or explicit save)
   ├─ CMPlayerCApp::StoreSettingsToRegistry()
   │   ├─ Open registry key
   │   ├─ Write each setting value
   │   └─ Close registry key
   │
   └─ OR CMPlayerCApp::StoreSettingsToIni()
       ├─ Open INI file
       ├─ Write each setting (section/key/value)
       └─ Close INI file
```

---

## 6. Performance Optimizations

### 6.1 Hardware Detection System

#### 6.1.1 Detection Process
```
1. CHardwareDetection::Initialize()
   ├─ Get system info (GetNativeSystemInfo)
   │   ├─ Number of processors
   │   ├─ Processor type
   │   └─ Page size
   ├─ Detect CPU features (CPUID)
   │   ├─ SSE2 support
   │   ├─ SSE3 support
   │   ├─ SSE4.1 support
   │   └─ AVX support
   ├─ Detect memory (GlobalMemoryStatusEx)
   │   ├─ Total physical RAM
   │   └─ Available physical RAM
   └─ Determine hardware tier
       ├─ LOW_END: < 1GB RAM OR (single core AND no SSE4.1)
       ├─ MID_RANGE: < 2GB RAM OR ≤ 2 cores
       └─ HIGH_END: Otherwise
```

#### 6.1.2 Adaptive Settings

**Low-End Hardware Optimizations:**
- Stream position poller interval: 200ms (default: 100ms)
- Timer intervals: 2000ms (default: 500ms)
- Disable seek preview
- Disable thumbnails
- Reduce UI updates
- Use lightweight renderer (if available)
- Priority class: NORMAL (avoid starving system)

**Mid-Range Hardware:**
- Stream position poller interval: 150ms
- Timer intervals: 1000ms
- Priority class: ABOVE_NORMAL

**High-End Hardware:**
- Use default settings
- Priority class: HIGH (if user allows)

### 6.2 Memory Optimizations

#### 6.2.1 Profile Caching
- Profile data cached in memory
- Lazy loading: Load on first access
- Flush on idle: Write to disk after inactivity
- Recursive mutex for thread safety

#### 6.2.2 Buffer Management
- Video decoder buffers: Adjusted based on available memory
- Audio buffer size: Configurable (default: 500ms)
- Subtitle buffer: Limited size for low-end systems

### 6.3 CPU Optimizations

#### 6.3.1 Timer Optimization
- Stream position poller: Adaptive interval based on hardware
- UI update timers: Reduced frequency on low-end hardware
- Stats timer: 1000ms (can be increased on low-end)

#### 6.3.2 Thread Priorities
- Main thread: Normal priority
- Graph thread: Normal priority (can be elevated for capture)
- UI thread: Normal priority
- Audio thread: Managed by audio renderer

#### 6.3.3 Polling Reduction
- Position polling: Adaptive interval
- External subtitle time: Higher frequency only when needed
- Stats updates: Reduced on low-end hardware

### 6.4 UI Optimizations

#### 6.4.1 Redraw Optimization
- Invalidate only changed regions
- Batch UI updates
- Defer non-critical updates

#### 6.4.2 Lazy Loading
- Toolbars: Load on first use
- Dialogs: Create on demand
- Thumbnails: Generate on demand (disabled on low-end)

### 6.5 Rendering Optimizations

#### 6.5.1 Renderer Selection
- Low-end hardware: Prefer EVR (lighter than VMR9)
- High-end hardware: Allow MPCVR/madVR (higher quality)

#### 6.5.2 Hardware Acceleration
- LAV Video: Use hardware decoding (DXVA2, D3D11) when available
- Video renderer: Use GPU acceleration when supported

---

## 7. Data Flow

### 7.1 Media Data Flow

```
Media File
    │
    ├─ Source Filter (LAV Splitter)
    │   ├─ Demuxes container
    │   ├─ Extracts video stream
    │   ├─ Extracts audio stream
    │   └─ Extracts subtitle stream
    │
    ├─ Video Path
    │   ├─ LAV Video Decoder
    │   │   ├─ Decodes video frames (H.264, HEVC, etc.)
    │   │   └─ Outputs uncompressed frames (YUV/RGB)
    │   │
    │   └─ Video Renderer (EVR/VMR9/MPCVR)
    │       ├─ Receives video frames
    │       ├─ Applies color adjustments
    │       ├─ Applies shaders (if enabled)
    │       └─ Renders to screen
    │
    ├─ Audio Path
    │   ├─ LAV Audio Decoder
    │   │   ├─ Decodes audio samples (AAC, AC3, etc.)
    │   │   └─ Outputs PCM audio
    │   │
    │   ├─ Audio Switcher (optional)
    │   │   ├─ Channel mapping
    │   │   ├─ Normalization
    │   │   └─ Volume/balance
    │   │
    │   └─ Audio Renderer (DirectSound/WASAPI/SaneAR)
    │       ├─ Receives audio samples
    │       ├─ Buffers audio
    │       └─ Plays to audio device
    │
    └─ Subtitle Path
        ├─ Subtitle Source Filter
        │   ├─ Parses subtitle file
        │   └─ Outputs subtitle entries
        │
        └─ Subtitle Renderer
            ├─ Receives subtitle entries
            ├─ Time-syncs with video
            └─ Renders text on video frame
```

### 7.2 Control Flow

```
User Input
    │
    ├─ Keyboard/Mouse → CMainFrame message handlers
    │   ├─ Play/Pause/Stop commands
    │   ├─ Seek commands
    │   ├─ Volume control
    │   └─ Menu commands
    │
    ├─ DirectShow Events → WM_GRAPHNOTIFY
    │   ├─ EC_COMPLETE (end of file)
    │   ├─ EC_ERRORABORT (error)
    │   └─ EC_REPAINT (repaint needed)
    │
    └─ Timer Events
        ├─ TIMER_STREAMPOSPOLLER (position updates)
        ├─ TIMER_STATS (statistics)
        └─ Other UI timers
```

### 7.3 Settings Flow

```
Settings Change
    │
    ├─ User modifies option in UI
    │   └─ Updates CAppSettings member
    │
    ├─ Application close / explicit save
    │   └─ CAppSettings::SaveSettings()
    │       ├─ Registry: WriteProfileInt/String
    │       └─ INI: Write to file
    │
    └─ Application startup
        └─ CAppSettings::LoadSettings()
            ├─ Registry: GetProfileInt/String
            └─ INI: Read from file
```

---

## 8. Configuration Management

### 8.1 Settings Storage

#### 8.1.1 Registry Storage
- **Location**: `HKEY_CURRENT_USER\Software\MPC-HC\MPC-HC`
- **Structure**: Hierarchical keys and values
- **Access**: Via CWinApp profile methods

#### 8.1.2 INI File Storage
- **Location**: Application directory or user-specified
- **File**: `mpc-hc.ini`
- **Format**: Standard INI format (sections, keys, values)
- **Advantage**: Portable, easy to edit

### 8.2 Settings Categories

1. **Player Settings** (`IDS_R_SETTINGS`)
   - Window position/size
   - Fullscreen options
   - UI theme
   - Toolbar configuration

2. **Playback Settings** (`IDS_R_SETTINGS`)
   - Speed control
   - Loop mode
   - After playback action
   - Fast seek

3. **Output Settings** (`IDS_R_SETTINGS`)
   - Video renderer
   - Audio renderer
   - Subtitle renderer

4. **Internal Filters** (`IDS_R_INTERNAL_FILTERS`)
   - Source filters
   - Transform filters
   - Renderer filters

5. **External Filters** (`IDS_R_EXTERNAL_FILTERS`)
   - User-configured filters
   - Filter priorities

### 8.3 Settings Migration

- **Registry → INI**: User can switch storage location
- **Version Migration**: Settings updated on version upgrade
- **Default Values**: Fallback if setting not found

---

## 9. Error Handling

### 9.1 Filter Graph Errors

#### 9.1.1 Graph Building Errors
- **Source Filter Not Found**: Try alternative source filters
- **Decoder Not Found**: Try alternative decoders
- **Connection Failed**: Report error, suggest alternatives
- **Renderer Not Available**: Fallback to default renderer

#### 9.1.2 Playback Errors
- **EC_ERRORABORT**: Stop playback, show error message
- **Decoder Errors**: Try software decoding if hardware fails
- **Renderer Errors**: Fallback to alternative renderer

### 9.2 File Access Errors

- **File Not Found**: Show error dialog
- **Access Denied**: Show error dialog with suggestion
- **Network Errors**: Retry with timeout

### 9.3 Exception Handling

- **C++ Exceptions**: Caught and logged
- **Structured Exceptions**: Crash reporter (if enabled)
- **Error Reporting**: User-friendly error messages

---

## 10. Hardware Detection & Optimization

### 10.1 Hardware Detection System

The hardware detection system (`CHardwareDetection`) automatically detects system capabilities and applies optimizations:

#### 10.1.1 Detection Criteria

**Low-End Hardware:**
- Total RAM < 1GB, OR
- Total RAM < 2GB AND single-core CPU, OR
- Single-core CPU AND no SSE4.1 support

**Mid-Range Hardware:**
- Total RAM < 2GB, OR
- ≤ 2 CPU cores

**High-End Hardware:**
- All other systems

#### 10.1.2 Optimizations Applied

**Low-End Hardware:**
- Increased stream position poller interval (200ms)
- Increased timer intervals (2000ms)
- Disabled seek preview
- Disabled thumbnails
- Reduced UI update frequency
- Normal priority class (to avoid starving system)
- Preference for lightweight renderers

**Mid-Range Hardware:**
- Moderate optimizations
- Above-normal priority class

**High-End Hardware:**
- Default settings
- High priority class (if user allows)

### 10.2 Performance Monitoring

- **Stream Position Polling**: Adaptive based on hardware
- **UI Update Frequency**: Reduced on low-end systems
- **Memory Usage**: Monitored and optimized
- **CPU Usage**: Thread priorities adjusted

### 10.3 User Override

Users can override automatic optimizations:
- Manual priority class setting
- Manual stream position poller interval
- Manual renderer selection
- Manual feature enable/disable

---

## Appendix A: Key Files Reference

### Core Application
- `src/mpc-hc/mplayerc.h/cpp`: Application class
- `src/mpc-hc/MainFrm.h/cpp`: Main window
- `src/mpc-hc/AppSettings.h/cpp`: Settings management
- `src/mpc-hc/BaseGraph.h/cpp`: Filter graph wrapper

### Filter Graph
- `src/mpc-hc/GraphThread.h/cpp`: Graph building thread
- `src/filters/`: Filter implementations
- `src/DSUtil/`: DirectShow utilities

### UI Components
- `src/mpc-hc/Player*.h/cpp`: Player UI components
- `src/mpc-hc/PPage*.h/cpp`: Settings pages
- `src/mpc-hc/CMPCTheme*.h/cpp`: Theme system

### Subtitle System
- `src/Subtitles/`: Subtitle processing
- `src/SubPic/`: Subtitle picture rendering

### Hardware Detection
- `src/mpc-hc/HardwareDetection.h/cpp`: Hardware detection and optimization

---

## Appendix B: DirectShow Interfaces Used

- **IGraphBuilder2**: Filter graph building
- **IMediaControl**: Playback control (Run/Pause/Stop)
- **IMediaSeeking**: Seeking and rate control
- **IMediaEventEx**: Event handling
- **IVideoWindow**: Video window management
- **IBasicVideo**: Video information
- **IBasicAudio**: Audio control (volume/balance)
- **IAMOpenProgress**: Opening progress
- **ISubPicAllocatorPresenter**: Subtitle rendering

---

## Document Version History

- **v2.0** (2024): Added hardware detection and low-end optimization documentation
- **v1.0**: Initial SRS document

---

**End of Document**

