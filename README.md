# lofihifi
win32 C/C++ trains neural net on wav spectral data to synthesize high frequencies for audio files that have none

you can download the .exe and .dat files and add high frequencies to your 44100 mono 16 bit riffusion recordings immediately -<br>
options > open data, select .dat (previously trained data file)<br>
file > open wav recorded from riffusion to enhance<br>
options > apply hifi<br>
file > save wav (or spacebar to play) - output wav is at 50% original amplitude plus highs<br>

note: you'll probably want to EQ the highs up as they are likely to be quiet, not much louder than the preexistent ones in riffusion presumably caused by windowing. they may suck. read on.<br>

it's recommended to train the model to music in the style you want to target for a few epochs, until the peaks are approximate.<br>
it's better for generated peaks to be under target than over. if peaks are varying above target, train on file with just bass sounds for an epoch or so to bias against peaks.<br>

the included .dat files are all pretty tame/overtrained in my tests, meaning the highs are quiet, but look like they follow alright,<br>
but you may get better variance starting from a less trained point. expect 30 epochs from new data to get spectral approximation without bad peaks.<br>

the included .dat files gradually saw magnitude error (the loudness, the important part) drop below 1 to even below .3, phase never got better than 5.5.<br>
i'd say once you're getting close to 1, and not seeing a lot of orange dots over 3dB higher than the blue dots, stop there for the most dynamic response. but i'm new here too.<br>

mainly i'm advanced enough not to be able to hear the frequencies this application targets, and this is my first neural net. i'm pretty sure you get improve<br>
your results by starting at a low training point and training with music in the target genre, using the bass trick a few epochs before finalising. but since you<br>
can adjust the learning rate during training, it makes more sense for someone who can hear to work on training. enjoy. it takes about 5 minutes an epoch on my computer.<br>
training time will not vary based on input wav file length. just use variance.. at first i got a constant noise floor because i didn't train on audio with spaces.<br>
have fun i figure training a net is like tuning a radio or cooking, wait and adjust prudently.<br>

if you are training and see a flat line, you forgot to load data/initialise a set of random data. if the orange generated data line disappears and the readout is full of NAN,<br>
it probably trained on a suddenly quiet passage or something with a lot of zeros across the spectrum. there's a safeguard against it but god knows what people might feed in so basically, hopefully you saved the data model beforehand.<br>
collect data files, build them and give them personalities. develop psychological attachments.<br>


there is a remove loops function.. it took three hours to go through a 4 minute file with a 2-5s range and found no loops. there maynot have been any, you know how riffusion can subtly change.<br>
sample length has to be precise, but there is a tolerance for recording line noise (i think around -50dB).<br>
