# Towel 808 Plugin

Towel 808 Plugin is an audio plugin that allows you to play and manipulate 808 bass samples with ease. Featuring a built-in sample selector, ADSR envelope controls, and a "Cut" function for immediate note cutoff, this plugin is perfect for producers looking to add deep bass tones to their tracks.

USE JUCE 6.1.6

<img width="607" alt="Screen Shot 2024-09-29 at 9 27 43 AM" src="https://github.com/user-attachments/assets/dd33141a-4d4b-45ac-8f56-38772d8603fe">

---

## Features

- **Sample Selector**: Choose from a variety of 808 bass samples.
- **ADSR Envelope Controls**: Customize Attack, Decay, Sustain, and Release settings.
- **Cut Function**: Enable immediate note cutoff when playing new notes.
- **MIDI Keyboard**: Built-in MIDI keyboard for quick testing and playback.

## Installation

### 1. Download the Plugin

- **macOS**: Download `Towel808Plugin.component` (AU) or `Towel808Plugin.vst3` (VST3).
- **Windows**: Download `Towel808Plugin.dll` (VST) or `Towel808Plugin.vst3` (VST3).

### 2. Install the Plugin

#### macOS

1. **AU Plugin**:
   - Copy `Towel808Plugin.component` to `/Library/Audio/Plug-Ins/Components/`.
2. **VST3 Plugin**:
   - Copy `Towel808Plugin.vst3` to `/Library/Audio/Plug-Ins/VST3/`.

#### Windows

1. **VST Plugin**:
   - Copy `Towel808Plugin.dll` to your VST plugins folder, typically `C:\Program Files\VstPlugins\`.
2. **VST3 Plugin**:
   - Copy `Towel808Plugin.vst3` to `C:\Program Files\Common Files\VST3\`.

### 3. Place the Sample Folder

**Important**: The plugin requires the `Towel Tuned 808s` folder to be placed in your Music directory to function properly.

#### macOS

- Copy the `Towel Tuned 808s` folder to:

  ```
  /Users/YourUsername/Music/Towel Tuned 808s
  ```

  Replace `YourUsername` with your actual username.

#### Windows

- Copy the `Towel Tuned 808s` folder to:

  ```
  C:\Users\YourUsername\Music\Towel Tuned 808s
  ```

  Replace `YourUsername` with your actual username.

### 4. Launch Your DAW

- Open your preferred Digital Audio Workstation (DAW).
- Perform a plugin rescan if necessary (check your DAW's documentation).
- Insert the Towel 808 Plugin onto a MIDI or instrument track.

## Usage

### Selecting a Sample

1. Open the plugin interface.
2. Use the **Sample Selector** dropdown menu at the top to choose an 808 sample.

### Adjusting the ADSR Envelope

- **Attack**: Controls how quickly the sound reaches full volume after a note is played.
- **Decay**: Determines the time it takes for the sound to decrease from the attack level to the sustain level.
- **Sustain**: Sets the level during the main sequence of the sound's duration.
- **Release**: Adjusts how long the sound fades out after the note is released.

To adjust these settings:

1. Move the sliders labeled **Attack**, **Decay**, **Sustain**, and **Release**.
2. Observe the changes in the sound as you play notes.

### Using the Cut Function

- **Cut Button**: When enabled, playing a new note will immediately stop the previous note, allowing for sharp transitions.

To use:

1. Click the **Cut** button to toggle the function on or off.
2. When **Cut** is active, notes will not overlap.

### Playing Notes

- **MIDI Keyboard Component**: Use the on-screen keyboard at the bottom of the plugin window.
- **External MIDI Controller**: Alternatively, play notes using your connected MIDI keyboard or controller.

## Troubleshooting

- **No Sound or Samples Not Loading**:
  - Ensure the `Towel Tuned 808s` folder is correctly placed in your Music directory.
  - Verify that the samples are in `.wav` format and are inside the `Towel Tuned 808s` folder.

- **Plugin Not Showing in DAW**:
  - Confirm that the plugin file is in the correct plugin folder.
  - Rescan plugins in your DAW's settings.

- **Error Messages About Missing Files**:
  - Double-check the spelling and placement of the `Towel Tuned 808s` folder.
  - Make sure your username in the file path is correct.
