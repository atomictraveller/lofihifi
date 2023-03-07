# lofihifi

you can download the .exe and the example.dat and hifi your files immediately..
options > open data, load example.dat (previously trained data file)\n
file > open wav recorded from riffusion to enhance\n
options > apply hifi\n
file > save wav (or spacebar to play)

you can train data on your own full spectrum audio files..<br>
i recommend combining full program audio with some single voice instruments to train simple phase relations -
file > open wav you want to train with
options > new data
options > train (for a few epochs. remember to save your data if you like it)

i will be (hopefully) improving the model in the next weeks. at that time, the model will be different and previous .dat files will not be compatible, but itm you can get some jangly high frequencies out of your riffusion recordings to EQ. i doubt the quality will ever be super good but we'll see how far i can get with this technique.

beware: the remove loops function is real slow. use short files.


source:

i'm an old C programmer i despise unnecessary complication (eg. everything modern). as far as i am concerned, source is a .c and a .h, i've only begrudgingly moved to msvc and have no intention of learning how to debug or use solutionn files or github's version control terminology or anything that didn't work the same twenty years ago.

the reason i'm saying this is my combat with github has already destroyed my ability to put files here from inside msvc, so the solution files have been placed manually, i have no idea if they will download and concoct a functional thingy.

but lofihifi.cpp, lofihifi.h, fft.h, and audio.h, are the only files i typed anything in except the two resource files for menu structure, so they're "source".

if you're on the riffusion discord i may enjoy any pointers towards completing the end user experience for you.
