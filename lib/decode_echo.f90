subroutine decode_echo(iwave,rxcall)

! Recovers the transmitted callsign from received EME echoes in EchoCall mode.
!     iwave(NZ)  received echo data
!     rxcall     decoded callsign
! Time alignment of received data is assumed accurate, since EME delay is known.

  parameter (NSPS=4096,NZ=6*NSPS)
  integer*2 iwave(NZ)      !Raw Rx data
  integer itone(6)         !Tone offsets corresponding to ransmitted callsign
  integer ipk(1)
  complex c0(0:NZ)                !Analytic data, 12000 Hz sample rate
  complex c1(0:NSPS-1)            !Data for a single echo character
  complex c2(0:NZ)                !Analytic data with shifted tone freqs
  character*6 rxcall              !The recovered callsign
  real s(0:NSPS-1)                !Spectrum for one received character
  real p(0:NSPS-1,6)
  real snr2(6)
  real a(3)
  character*37 c
  common/echocom/nclearave,nsum,blue(4096),red(4096)
  equivalence (ipk1,ipk(1))
  data c/' 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ'/

! Retrieve some information known at the time of transmissiion
  call save_echo_params(nDop,nDopAudio,nfrit,f1,fspread,ndf,itone,iwave,-1)

  df=12000.0/NSPS
  if(nclearave.ne.0) p=0.
  nfft=NZ
  df1=12000.0/nfft
  fac=2.0/(32767.0*nfft)
  c0(0:NZ-1)=fac*iwave(1:NZ)
  call four2a(c0,nfft,1,-1,1)             !Forward c2c FFT
  c0(nfft/2+1:nfft-1)=0.
  c0(0)=0.5*c0(0)
  call four2a(c0,nfft,1,1,1)              !Inverse c2c FFT; c0 is analytic sig

  rxcall='      '
  i1=nint((f1 - 5*ndf)/df)
  i2=nint((f1 + 42*ndf)/df)
  nn=i2-i1+1
  nskip=2*fspread/df
  nerr=0

  do j=1,6
     ia=(j-1)*NSPS
     ib=ia+NSPS-1
     c1=c0(ia:ib)
     call four2a(c1,NSPS,1,-1,1)           !Forward c2c
     do i=0,NSPS/2
        s(i)=real(c1(i))**2 + aimag(c1(i))**2
     enddo
     call smo121(s,NSPS/4)
     call smo121(s,NSPS/4)
     p(:,j)=p(:,j) + s                     !Sum the spectra for each character
     ipk=maxloc(p(i1:i2,j))
     k=nint(((ipk1+i1-1)*df - f1)/ndf) + 1
     call averms(p,nn,nskip,ave,rms)
     spk=maxval(p(i1:i2,j))
     snr=(spk-ave)/rms
     snr2(j)=snr
     if(k-1-itone(j).ne.0) nerr=nerr+1
!     write(61,3001) ave,rms,spk,snr,j,k-1,itone(j),abs(k-1-itone(j))
!3001 format(4f8.3,4i5)
!     do i=i1,i2
!        write(62,3062) i*df,p(i,j),ave,rms,snr
!3062    format(2f10.3,3f8.2)
!     enddo
     if(k.ge.1 .and. k.le.37) rxcall(j:j)=c(k:k)    !SNR test here ???
  enddo

!  write(*,4001) snr2
!4001 format(6f7.1)

!  call echo_snr(p,p,fspread,blue,red,snrdb,db_err,dfreq,snr_detect)
!  write(*,3101) fspread,snrdb,db_err,dfreq,snr_detect
!3101 format(5f10.3)

  do j=1,6
     ia=(j-1)*NSPS
     ib=ia+NSPS-1
     a=0.
     a(1)=-itone(j)*ndf
     call twkfreq(c0(ia:ib),c2(ia:ib),NSPS,12000.0,a)
  enddo
  nfft=32768
  call four2a(c2,nfft,1,-1,1)           !Forward c2c
  do i=0,8192
     f=i*12000.0/32768
     sq=real(c2(i))**2 + aimag(c2(i))**2
     write(54,3012) f,sq
3012 format(f10.3,e12.3)
  enddo
!  print*,'AAA',nn,nskip,nerr
  
  return
end subroutine decode_echo
