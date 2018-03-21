// show signal
// close all figures
xdel(winsid());
clear;    // delete all variables

pathname = get_absolute_file_path("filterdesign.sce")
chdir(pathname);

fs = 16000
samples = 50
t = 0:1/fs:1/fs*(samples-1)
f = 1000
left = 1*squarewave(2*%pi*f*t);

f2 = 100
left2 = 1*squarewave(2*%pi*f2*t);

plot(t,left,'-bo')
plot(t,left2,'-go')

firTaps = 128;

[h,hm,frfir]=wfir("lp",firTaps,[f*0.8/fs f*1.2/fs],"hn",[0 0])
hz=iir(2,'lp','cheb2',[2*f/fs f*1.2/fs],[.2 .2]);
[hzm,fr]=frmag(hz,256);


scf();
//plot2d('nl',fr,hzm,4)
plot2d('nl',frfir,hm,3)
klh
//get the coefficients from the 
iirb = coeff(hz(2));
iira = coeff(hz(3));
// invert the order of coefficients
a=iira(length(iira):-1:1);
b=iirb(length(iirb):-1:1);


leftIfil=filter(b,a,left)
leftIfil=filter(b,a,leftIfil)
//leftIfil=filter(hz(2),hz(3),leftIfil,zeros(1,length(iirb)-1))

//leftIfil=filter(h,1,left)
leftIfil2=filter(h,1,left2)

//rightIfil=filter(h,1,rightInterpolated)

scf();
plot(leftIfil,'-b')
//plot(leftIfil2,'-go')
plot(left,'-g')

a = round(a.*(2^15-1));
b = round(b.*(2^15-1));
hround = round(h.*(2^15-1));

fd = mopen('coeff.h','wt');
mfprintf(fd,'q15_t iirCoeff[12] = {\n');
mfprintf(fd,'%d, %d, %d, %d, %d, %d,\n', b(1), 0, b(2), b(3), a(2), a(3));   // first stage
mfprintf(fd,'%d, %d, %d, %d, %d, %d\n', b(1), 0, b(2), b(3), a(2), a(3));   // second stage
mfprintf(fd,'};\n');
mfprintf(fd,'q15_t firCoeff[%d] = {\n', firTaps);
for i=1:firTaps
    mfprintf(fd,'%d, ', hround(i));   // first stage
end

mfprintf(fd,'};\n');
mclose(fd);




