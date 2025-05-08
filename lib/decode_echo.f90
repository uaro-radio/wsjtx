subroutine decode_echo(txcall,iwave,rxcall)

  parameter (NSPS=4096,NH=NSPS/2,NZ=3*12000)
  character*6 txcall,rxcall
  integer*2 iwave(NZ)
  integer*2 id2a(12)

  integer itone(6)
  integer ipk(1)
  complex c0(NZ)                         !Analytic data, 6000 Hz sample rate
  complex c1(0:NH-1)
  real s(0:NSPS-1),p(0:NSPS-1)
  real p2(0:NSPS-1,6)
  character*37 c
  common/echocom/nclearave,nsum,blue(4096),red(4096)

  equivalence (nDopTotal0,id2a(1))
  equivalence (nDopAudio0,id2a(3))
  equivalence (nfrit0,id2a(5))
  equivalence (f10,id2a(7))
  equivalence (fspread0,id2a(9))
  equivalence (ntonespacing0,id2a(11))

  data c/' 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ'/

  id2a=iwave(1:12)
  iwave(1:12)=0
  if(ntonespacing0.ne.5 .and. ntonespacing0.ne.10 .and.                    &
       ntonespacing0.ne.20 .and. ntonespacing0.ne.50) ntonespacing0=10
  dftone=ntonespacing0
  
  itone=0                                               !Default character is blank
  do i=1,len(trim(txcall))
     m=ichar(txcall(i:i))
     if(m.ge.48 .and. m.le.57) itone(i)=m-47       !0-9
     if(m.ge.65 .and. m.le.90) itone(i)=m-54       !A-Z
     if(m.ge.97 .and. m.le.122) itone(i)=m-86      !a-z
  enddo

  df=6000.0/NH
  if(nclearave.ne.0) p2=0.

!  write(*,3001) nDopTotal0,nDopAudio0,nfrit0,ntonespacing0,f10,fspread0
!3001 format(4i6,2f7.1)
  iwave(1:12)=0
  call ana64(iwave,NZ,c0)
  p=0.
  rxcall='      '
  do j=1,6
     ib=j*NH
     ia=ib-NH+1
     c1=c0(ia:ib)
     call four2a(c1,NH,1,-1,1)           !Forward c2c
     do i=0,NH-1
        s(i)=real(c1(i))**2 + aimag(c1(i))**2
        write(52,3012) i*df,s(i)
     enddo
     n=nint(itone(j)*dftone/df)
     p=p+cshift(s,n)
     p2(:,j)=p2(:,j)+s
!        ipk=maxloc(s)
     ipk=maxloc(p2(:,j))
     k=nint(((ipk(1)-1)*df - 1500.0)/dftone) + 1
     if(k.ge.1 .and. k.le.37) rxcall(j:j)=c(k:k)
  enddo

  do i=0,NSPS/2
     write(53,3012) i*df,p(i)
3012 format(f10.3,e12.3)
  enddo

  return
end subroutine decode_echo
