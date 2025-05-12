program testEchoCall

  parameter (NSPS=4096,NH=NSPS/2,NZ=3*12000)
  integer*2 iwave(NZ)                    !Raw data, 12000 Hz sample rate
  integer ihdr(11)
  integer*1 itone(6)
  character*120 fname
  character*6 rxcall
  common/echocom/nclearave,nsum,blue(4096),red(4096)

  narg=iargc()
  if(narg.lt.1) then
     print*,'Usage: testEchoCall fname1 [fname2, ...]'
     go to 999
  endif

  write(*,1000)
1000 format(' N   Rcvd'/11('-'))

  nclearave=1
  do ifile=1,narg
     call getarg(ifile,fname)
     open(10,file=trim(fname),access='stream',status='unknown')
     read(10) ihdr,iwave
     close(10)
     call decode_echo(iwave,rxcall)
     write(*,1100) ifile,rxcall
1100 format(i3,2x,a6)
     nclearave=0
  enddo

999 end program testEchoCall
