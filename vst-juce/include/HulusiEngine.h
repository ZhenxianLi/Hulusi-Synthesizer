#pragma once

#include <array>
#include <algorithm>
#include <cmath>

class HulusiEngine
{
public:
    void prepare(double sampleRate)
    {
        fs = sampleRate;
        phase.fill(0.0f);
        env = 0.0f;
        vibPhase = 0.0f;
    }

    void setParams(float gainIn, float breathIn, float vibDepthIn, float vibRateIn)
    {
        gain = gainIn;
        breath = breathIn;
        vibDepth = vibDepthIn;
        vibRate = vibRateIn;
    }

    void noteOn(int midiNoteIn, int velocityIn)
    {
        midiNote = midiNoteIn;
        velocity = std::clamp(velocityIn, 1, 127);
        gate = true;
    }

    void noteOff(int midiNoteIn)
    {
        if (midiNoteIn == midiNote)
            gate = false;
    }

    float renderSample()
    {
        if (fs <= 0.0)
            return 0.0f;

        env = gate ? std::min(1.0f, env + 0.0025f) : std::max(0.0f, env - 0.0020f);

        const auto f0 = midiToFrequency(midiNote);
        const auto harm = interpolatedHarmonics(f0);

        vibPhase += static_cast<float>(2.0 * M_PI * vibRate / fs);
        const float vib = 1.0f + vibDepth * std::sin(vibPhase);

        float sample = 0.0f;
        for (size_t k = 0; k < phase.size(); ++k)
        {
            const float fk = static_cast<float>(f0 * (k + 1) * vib);
            phase[k] += static_cast<float>(2.0 * M_PI * fk / fs);
            sample += harm[k] * std::sin(phase[k]);
        }

        // very light deterministic pseudo-noise (no RNG state shared with host)
        noiseState = 1664525u * noiseState + 1013904223u;
        const float n = ((noiseState >> 8) & 0xFFFF) / 32768.0f - 1.0f;
        const float breathNoise = breath * n * 0.12f;

        return gain * env * (sample + breathNoise) * std::max(velocity / 127.0f, 0.1f);
    }

private:
    static constexpr std::array<double, 10> noteFreqTable {
        329.6276,391.9954,440.0000,493.8833,523.2511,587.3295,659.2551,698.4565,783.9909,880.0000
    };

    static constexpr std::array<std::array<float, 10>, 20> harmTable {{
        {{280.5516875f,145.9560279f,463.6630936f,461.5251985f,269.8331318f,388.1711321f,830.0093149f,816.44568f,702.4411407f,1094.3393793f}},
        {{78.56941688f,247.1768507f,110.7097929f,106.8131165f,102.8653057f,14.67236534f,11.53680331f,6.99955669f,13.14478614f,31.95926832f}},
        {{288.3316496f,8.568150795f,77.90825359f,219.81859f,200.257204f,390.1042718f,27.90990969f,17.67879058f,44.63662807f,78.59943173f}},
        {{46.64692576f,31.26524547f,115.2608189f,18.44744541f,11.49521112f,28.32804914f,10.34977931f,8.395348992f,0.0f,8.987842658f}},
        {{14.87293657f,11.69877742f,13.52944326f,23.71767889f,23.59219387f,31.06338926f,4.658243733f,0.0f,4.035377987f,16.30525803f}},
        {{7.974578965f,148.6351444f,12.06983844f,7.847422507f,28.32876759f,0.0f,0.0f,0.0f,0.0f,0.0f}},
        {{15.21359081f,40.29874784f,18.64485006f,7.310882075f,3.696508376f,2.757218469f,0.0f,0.0f,0.0f,0.0f}},
        {{8.652610586f,38.78241986f,10.22704945f,3.501782095f,9.56719708f,2.076658832f,0.0f,0.0f,0.0f,0.0f}},
        {{11.28211234f,71.16270898f,3.738239731f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f}},
        {{0.0f,6.365706743f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f}},
        {{2.32140946f,4.925496831f,4.161408557f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f}},
        {{0.0f,1.240415649f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f}},
        {{0.0f,4.655299521f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f}},
        {{0.0f,3.526926293f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f}},
        {{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f}},
        {{0.0f,1.397586f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f}},
        {{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f}},
        {{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f}},
        {{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f}},
        {{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f}}
    }};

    static double midiToFrequency(int midi)
    {
        return 440.0 * std::pow(2.0, (midi - 69) / 12.0);
    }

    static float log2Hz(double hz)
    {
        return static_cast<float>(std::log(hz) / std::log(2.0));
    }

    static std::array<float, 20> interpolatedHarmonics(double freq)
    {
        const float q = log2Hz(std::max(freq, 1.0));
        std::array<float, 20> out {};

        for (size_t h = 0; h < out.size(); ++h)
        {
            out[h] = interpolateRow(harmTable[h], q);
            out[h] = std::max(out[h], 0.0f);
        }

        float sum = 0.0f;
        for (auto v : out) sum += std::abs(v);
        if (sum > 0.0f)
            for (auto& v : out) v /= sum;

        return out;
    }

    static float interpolateRow(const std::array<float, 10>& row, float query)
    {
        std::array<float, 10> x {};
        for (size_t i = 0; i < x.size(); ++i)
            x[i] = log2Hz(noteFreqTable[i]);

        if (query <= x.front())
            return linear(x[0], row[0], x[1], row[1], query);
        if (query >= x.back())
            return linear(x[8], row[8], x[9], row[9], query);

        for (size_t i = 0; i < x.size() - 1; ++i)
            if (query >= x[i] && query <= x[i + 1])
                return linear(x[i], row[i], x[i + 1], row[i + 1], query);

        return row.back();
    }

    static float linear(float x0, float y0, float x1, float y1, float x)
    {
        const float t = (x - x0) / (x1 - x0 + 1.0e-12f);
        return y0 + t * (y1 - y0);
    }

    double fs = 44100.0;
    float gain = 0.6f;
    float breath = 0.12f;
    float vibDepth = 0.003f;
    float vibRate = 5.0f;

    int midiNote = 69;
    int velocity = 64;
    bool gate = false;

    float env = 0.0f;
    float vibPhase = 0.0f;
    std::array<float, 20> phase {};
    unsigned int noiseState = 1u;
};
