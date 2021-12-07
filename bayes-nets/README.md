This folder contains the coded versions of various Bayes Nets. Many are not in 
a working condition. Currently (as of 5/2019), I (Simon) have moved away from 
using libraries due to the baggage and restrictions they bring. It's harder to 
understand when and why things go wrong because they make a lot of assumptions 
and design decisions under the hood. The folders demand an explanation:

-python - models coded using native Python, numpy, and scipy. These do not use 
    any Bayes Net libraries.
-pgmpy - models using the Python library, pgmpy.
-stan - models using the Stan modeling language.

Usually they're named pretty descriptively so you can tell what they are doing.
