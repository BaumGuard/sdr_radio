# sdr_radio
A small C program to receive and demodulate **AM** and **FM** radio using an SDR.

I developed it as a nice introduction to learn about digital signal processing with demodulation and filters.

## Supported SDRs
Currently the following types of SDRs are supported
- RTL-SDR
- AirspyHF+

## Dependencies
For Ubuntu
- `librtlsdr-dev`
- `libairspyhf-dev`
- `libasound2-dev`

For Arch Linux
- `rx_tools`
- `airspy`
- `airspyhf-git`
- `alsa-lib`

## Compilation
First, install the dependencies mentioned above in case they are not already installed on your system.

Create the `build` directory
```bash
mkdir build
```

Build the program using `make`
```bash
make
```

## Usage
Connect the SDR to the USB port of your computer and launch `sdr_radio` with the frequency of the radio station you would like to listen to
```bash
./sdr_radio -f <frequency>
```

You should now hear the radio playing over your speakers.

If the frequency is higher than 30 MHz, `sdr_radio` will automatically perform FM demodulation and AM demodulation in the opposite case.

### Commandline parameters
`sdr_radio` also offers the following commandline parameters:
<table>
    <tr>
        <th>Parameter</th>
        <th>Usage</th>
    </tr>
    <tr>
        <td><code>-f &lt;frequency></code&gt;</td>
        <td>
            Set the receiving frequency
            <ul>
                <li>Whole numbers (e.g. <code>100700000</code>) specifiy the frequency in Hz)</li>
                <li>Whole/floating point numbers with an <code>M</code> attached (e.g. <code>100.7M</code>) specify the frequency in MHz</li>
                <li>Whole/floating point numbers with a <code>k</code> attached (e.g. <code>5625k</code>) specify the frequency in kHz</li>
            </ul>
        </td>
    </tr>
    <tr>
        <td><code>-r &lt;path-to-wav-file></code&gt;</td>
        <td>
            Record the audio stream in a WAV file
        </td>
    </tr>
</table>


## Technical stuff (How it works)
The program receives a stream of IQ sample packets delivered by the SDR and processes the samples sequentially in the following steps.

### FM
1. **Demodulation**
    Demodulate the signal by calculating the phase of every IQ sample pair
    $$\phi_0 = atan2(q, i)$$
    
    Calculate the phase difference to the previous phase
    $$\Delta \phi = \phi_0 - \phi_{-1}$$
    
    If the phase difference is not between $-\pi$ and $\pi$
    $$\Delta \phi = \begin{cases} -\pi \le \Delta \phi \le \pi, & \Delta \phi \\\Delta \phi < -\pi, & \Delta \phi + \pi \\ \Delta \phi > \pi, & \Delta \phi - \pi \end{cases}$$
    
2. **Low pass filtering**
    The low pass filter is implemented as a second degree IIR Butterworth filter with a cutoff frequency of 17 kHz. The filter lets the audio channel pass through and cancels out the higher frequencies to prevent aliasing.
    
3. **Downsampling**
    Since the SDR samples the signal at a frequency of 240000 Hz (at least in case of an RTL-SDR), but audio normally has a sampling frequency of only 48000 Hz, it is necessary to downsample the signal to output it over the speakers. This is done by basically using only every fifth sample, since 240000 Hz divided by 48000 Hz is 5, and discarding the remaining samples.

4. **Audio output**
    `sdr_radio` uses Linux's standard audio server *ALSA* to output the processed audio data through the speakers. After having collected 1024 samples, the samples are sent to the *ALSA* interface in order to output them.
    
### AM
1. **Demodulation**
    Demodulate the signal by calculating the magnitude of every IQ sample pair and normalizing it to a range of -1 to 1.
    $$mag = \frac{\sqrt{i^2 + q^2}}{\sqrt{2}}$$
    
2. **Low pass filtering**
    The low pass filter is implemented as a second degree IIR Butterworth filter with a cutoff frequency of 5 kHz. The filter lets the audio channel pass through and cancels out the higher frequencies to prevent aliasing.
    
The **downsampling** and the **audio output** works the same as for **FM**.
