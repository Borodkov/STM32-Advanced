// show signal
// close all figures
xdel(winsid());
clear;    // delete all variables

realpath = cd()
pathname = get_absolute_file_path("scilabProcess1.sce")
chdir(pathname);

len = 512;
speedOfSound = 340;
fs= 16000;

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

t = (0: 1/fs: (len - 1) * 1/fs);    // generate real time scale

plot(t, left, "g");
plot(t, right, "r");
xtitle('left and right channel amplitude', 'time', 'amplitude (-)');
hl=legend(['left';'right']);
