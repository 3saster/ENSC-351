#!/bin/sh

python -c "import random; my_randoms = random.sample(xrange(2**32-1), 10**4); sarr = [str(a) for a in my_randoms]; print ' '.join(sarr)" > prime.txt
