# Kick-starter

In this (`csc`) directory, simply `make`. You need a cross-compiler, or you can compile on the target (probably). Run any `start_*` to start demo trace session. To parse the generated trace data, you need first the `deformat`, and second the `ETM_data_parser`. 

## Technical Details

CoreSight Trace is non-trivial, especially one desire for advanced features. After years of digging, we present insights/thinkModel/useGuide here. The primary focuses is on Embedded Trace Macrocell (ETM).

### Resource

Advanced features in ETM usually involve Trace Unit Resource. ETM supports up to 32 such resources. As for Cortex-A53, it implements 16 resources. It is useful to think each resource as a **single-input multi-output** entity. When the entity receives an input, it can assert all its output. What can be an input signal? A range of trace unit events, this includes but not limited to:

- program counter hits an user-defined virtual address.
- the PID changes to an user-defined value.
- an external input occurs.
- the counter on ETM reaches zero.

You can use the **Resource Selection Control Register** to control which input the entity listens to. Now we answer who can use the output from the entity. When look up the register description in [ETM specification](https://developer.arm.com/documentation/ihi0064/latest/), whenever you see a 8-bit field, **event selector**, presents in a register, it indicates its underlying component can listen to an entity output. For example, the **counter control register** contains the 8-bit event selector field. You can then write to the field, so that the underlying counter will listen to the entity the field indicates. The manual will tell you how the counter reacts to the entity output: in this case, the counter value decrements by one. 

Let's push the example further. Assuming the above mentioned entity is _R2_, we now program another entity _R3_ by writting to its resource selection control register. We let **the counter reaching zero** be the input of _R3_. Thus the net result is that _R3_ will fire after a defined number inputs received by _R2_. This can be used to express semantics such as "R3 fires when the virtual address 0x400000 is hit by program counter every ten times". 

### External Input Event

Performance Monitor Unit (PMU) often offers valuable statistics regarding the processes by monitoring the architectural events. When ETM presents, PMU can also signal ETM when certain architectural event occurs. Notice architectural events monitored by PMU (such as instruction retired, L2 data cache refill, etc...) are not ETM event. How to let ETM listen to the architectural events? You are right! the resource can be programmed to choose a specific architectural event to be its input, by programming the resource selection control register to External Input group. Precisely, an additional register **external input selector** also involves. Check the manual. 

### ETM asserts external output

When an resource asserts its output, ETM can also asserts ETM itself's **External Output Pins**. What hardware the ETM output pins pointing to? This is usually hard-wired by the vendor. Check the manual. But probably some Cross-Trigger Interface (CTI) for better flexibiliy. How to let ETM delivery the output? You guessed correctly! There is an 8-bit event selector field for it. Specifically, the **Event Control Register 0** provides four event selector fields, each can be programmed to listen to a resource. By doing so, when the resource fires, ETM will also assert its corresponding external output pin. 

Additionally, **Event Control Register 1** also allows the fire of the resource to be reported in the trace stream in form of event packets. 

## Conclusion

With the examples and write-up presented here, I believe you can navigate the ETM manual relatively with ease. If you feel this repo is helpful, please Star it and [cite our paper](https://drops.dagstuhl.de/entities/document/10.4230/LIPIcs.ECRTS.2023.13)! Thanks! 笔芯
