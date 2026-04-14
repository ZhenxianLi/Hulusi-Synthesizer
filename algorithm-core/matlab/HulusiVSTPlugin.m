classdef HulusiVSTPlugin < audioPlugin
    % HulusiVSTPlugin
    % 一个基于项目谐波分析结果的简易单声部葫芦丝VST插件（MATLAB Audio Toolbox）

    properties
        Gain (1,1) double {mustBeNonnegative, mustBeFinite} = 0.6
        Breath (1,1) double {mustBeNonnegative, mustBeFinite} = 0.12
        VibratoDepth (1,1) double {mustBeNonnegative, mustBeFinite} = 0.003
        VibratoRate (1,1) double {mustBePositive, mustBeFinite} = 5.0
    end

    properties (Constant)
        PluginInterface = audioPluginInterface( ...
            audioPluginParameter('Gain','DisplayName','Gain','Label','linear','Mapping',{'lin',0,1}), ...
            audioPluginParameter('Breath','DisplayName','Breath','Label','mix','Mapping',{'lin',0,0.5}), ...
            audioPluginParameter('VibratoDepth','DisplayName','Vibrato Depth','Label','ratio','Mapping',{'lin',0,0.02}), ...
            audioPluginParameter('VibratoRate','DisplayName','Vibrato Rate','Label','Hz','Mapping',{'lin',0.1,12}) ...
        )
    end

    properties (Access=private)
        Fs (1,1) double = 44100
        CurrentNote (1,1) double = 69
        Velocity (1,1) double = 0
        Phase (20,1) double = zeros(20,1)
        VibratoPhase (1,1) double = 0
        Envelope (1,1) double = 0
        IsGateOn (1,1) logical = false
        HarmScale (20,10) double
        NoteFreqTable (10,1) double
    end

    methods
        function obj = HulusiVSTPlugin()
            obj.NoteFreqTable = [329.6276;391.9954;440.0000;493.8833;523.2511;587.3295;659.2551;698.4565;783.9909;880.0000];
            obj.HarmScale = localHarmScaleTable();
        end

        function reset(obj)
            obj.Fs = getSampleRate(obj);
            obj.Phase(:) = 0;
            obj.VibratoPhase = 0;
            obj.Envelope = 0;
        end

        function out = process(obj, in)
            numSamples = size(in,1);
            if numSamples == 0
                out = in;
                return;
            end

            f0 = localMidiToFrequency(obj.CurrentNote);
            harm = localInterpolatedHarmonics(f0, obj.NoteFreqTable, obj.HarmScale);
            harm = harm / max(sum(abs(harm)), eps);

            y = zeros(numSamples,1);
            for n = 1:numSamples
                if obj.IsGateOn
                    obj.Envelope = min(1, obj.Envelope + 0.0025);
                else
                    obj.Envelope = max(0, obj.Envelope - 0.0020);
                end

                obj.VibratoPhase = obj.VibratoPhase + 2*pi*obj.VibratoRate/obj.Fs;
                vib = 1 + obj.VibratoDepth * sin(obj.VibratoPhase);

                sample = 0;
                for k = 1:20
                    fk = f0 * k * vib;
                    obj.Phase(k) = obj.Phase(k) + 2*pi*fk/obj.Fs;
                    sample = sample + harm(k)*sin(obj.Phase(k));
                end

                breathNoise = obj.Breath * randn(1,1) * 0.15;
                y(n) = obj.Gain * obj.Envelope * (sample + breathNoise) * max(obj.Velocity/127, 0.1);
            end

            if size(in,2) == 1
                out = y;
            else
                out = [y y];
            end
        end

        function processMIDI(obj, midi)
            for idx = 1:numel(midi)
                msg = midi(idx);
                if msg.IsNoteOn
                    obj.CurrentNote = msg.Note;
                    obj.Velocity = msg.Velocity;
                    obj.IsGateOn = true;
                elseif msg.IsNoteOff
                    if msg.Note == obj.CurrentNote
                        obj.IsGateOn = false;
                    end
                end
            end
        end
    end
end

function freq = localMidiToFrequency(midiNote)
refMidi = 69;
freq = 440 * 2.^((midiNote - refMidi)/12);
end

function harm = localInterpolatedHarmonics(freq, noteFreqTable, harmScaleTable)
% 在 log2 频率轴上插值，使 10 个实测音高可扩展到完整音域
logTable = log2(noteFreqTable(:));
query = log2(max(freq, eps));
harm = zeros(size(harmScaleTable, 1), 1);

for k = 1:size(harmScaleTable, 1)
    row = harmScaleTable(k, :).';
    harm(k) = interp1(logTable, row, query, 'linear', 'extrap');
end

% 避免插值外推导致负增益
harm = max(harm, 0);
end

function table = localHarmScaleTable()
table = [ ...
    280.551687532203,145.956027902726,463.663093646505,461.525198485626,269.833131802056,388.171132062961,830.009314852516,816.445679995814,702.441140667890,1094.339379313690; ...
    78.569416884089,247.176850699589,110.709792939480,106.813116505853,102.865305696257,14.672365336415,11.536803314655,6.999556690389,13.144786139192,31.959268320383; ...
    288.331649592183,8.568150794516,77.908253589094,219.818590007465,200.257204013202,390.104271830325,27.909909690140,17.678790579205,44.636628072188,78.599431729863; ...
    46.646925755160,31.265245467371,115.260818884912,18.447445411930,11.495211123512,28.328049136371,10.349779306817,8.395348991841,0,8.987842657563; ...
    14.872936566982,11.698777419727,13.529443259248,23.717678892303,23.592193873401,31.063389264488,4.658243733370,0,4.035377986747,16.305258032294; ...
    7.974578964847,148.635144411488,12.069838441056,7.847422506956,28.328767586518,0,0,0,0,0; ...
    15.213590809525,40.298747842116,18.644850055509,7.310882074961,3.696508376223,2.757218469161,0,0,0,0; ...
    8.652610585686,38.782419856160,10.227049450741,3.501782094988,9.567197079757,2.076658832075,0,0,0,0; ...
    11.282112337632,71.162708976933,3.738239730885,0,0,0,0,0,0,0; ...
    0,6.365706743482,0,0,0,0,0,0,0,0; ...
    2.321409460425,4.925496831346,4.161408556600,0,0,0,0,0,0,0; ...
    0,1.240415649235,0,0,0,0,0,0,0,0; ...
    0,4.655299520763,0,0,0,0,0,0,0,0; ...
    0,3.526926293446,0,0,0,0,0,0,0,0; ...
    0,0,0,0,0,0,0,0,0,0; ...
    0,1.397585999495,0,0,0,0,0,0,0,0; ...
    0,0,0,0,0,0,0,0,0,0; ...
    0,0,0,0,0,0,0,0,0,0; ...
    0,0,0,0,0,0,0,0,0,0; ...
    0,0,0,0,0,0,0,0,0,0 ...
    ];
end
