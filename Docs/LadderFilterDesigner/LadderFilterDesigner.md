# Ladder Filter Designer

## Introduction

Over the past few year I have implemented a few multimode ladder filters in both 
software and hardware, each time I have learned something, and wished I had included 
that knowledge in previous designs. Recently I have been building a Diy modular synth 
where I have discovered the joys of both non linear signal paths, and the variations
caused by the component tolerances, no two builds of my filter have sounded the same.

![alt text] (../../images/Wallenda.png "Wallander")

## How multimode pole mixing ladder filters work

There are many ways to implement a filter, but the method I use and first discovered
while reading "Designing Software Synthesizers" is a ladder filter, where the outputs
of each stage are summed with varing weights. Each pole of a ladder filter is a 6dB 
lowpass filter.

The multimode is achieved because it is possiable to sum the stages. The following
image, simply shows the process.

![alt text] (https://electricdruid.net/wp-content/uploads/2017/04/Highpass.png "Filter summing")

A simple ladder filter with resonance can be constructed by following the diagram




## Non linear 

## Tolerances in electronic implementations 

## Oversampling

## VCV Rack modules(s)

## References, additional reading

Designing Software Synthesizer Plug-ins in C++, Pirkle, 2015, https://www.amazon.co.uk/Designing-Software-Synthesizer-Plug-Ins-RackAFX/dp/1138406449

Designing Audio Effect Plugins in C++, Pirkle, 2019, https://www.amazon.co.uk/Designing-Audio-Effect-Plugins-Theory/dp/1138591939/

Multimode filters, Part 1: Reconfigurable filters  https://electricdruid.net/multimode-filters-part-1-reconfigurable-filters/


