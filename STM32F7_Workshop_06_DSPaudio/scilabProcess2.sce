// show signal
// close all figures
xdel(winsid());
clear;    // delete all variables

realpath = cd()
pathname = get_absolute_file_path("scilabProcess2.sce")
chdir(pathname);

len = 512;
speedOfSound = 340;
fs= 16000;

t = 0:1/fs:1/fs*(len-1)
f = 1000

// convert hex to bin with srec_cat freeware utility
host("srec_cat left.hex -intel -offset - -minimum-addr left.hex -intel -o left.bin -binary")
host("srec_cat right.hex -intel -offset - -minimum-addr right.hex -intel -o right.bin -binary")
host("srec_cat corr.hex -intel -offset - -minimum-addr corr.hex -intel -o corr.bin -binary")
host("srec_cat leftFFT.hex -intel -offset - -minimum-addr leftFFT.hex -intel -o leftFFT.bin -binary")


fd_rb = mopen('left.bin','rb');
left = mget(len, 's', fd_rb);
mclose(fd_rb);

fd_rb = mopen('right.bin','rb');
right = mget(len, 's', fd_rb);
mclose(fd_rb);

fd_rb = mopen('corr.bin','rb');
stm32Corr = mget(2*len-1, 's', fd_rb);
mclose(fd_rb);

plot(left, "g");
plot(right, "r");
xtitle('left and right channel amplitude', 'time', 'amplitude (-)');
hl=legend(['left';'right']);
scf();
subplot(211);
plot(stm32Corr, "r");
xtitle('cross corelation in STM32', 'time', 'amplitude (-)');
[maxCorr,maxCorrIndex] = max(stm32Corr);
delayStm32 = maxCorrIndex - len - 1;

// now recompute by SciLab
scilabCorr = xcorr(left, right);
subplot(212);
xtitle('cross corelation in Scilab', 'time', 'amplitude (-)');
plot(scilabCorr, "g");
[maxCorr,maxCorrIndex] = max(scilabCorr);
delaySciLab = maxCorrIndex - len - 1;


