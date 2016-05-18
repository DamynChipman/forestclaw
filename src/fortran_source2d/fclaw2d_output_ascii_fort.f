      subroutine fclaw2d_fort_write_header(matname1,matname2,
     &      time,meqn,ngrids)
      implicit none

      integer iframe,meqn,ngrids

      character*10 matname1
      character*10 matname2
      double precision time
      integer matunit1, matunit2, nstp,ipos,idigit

      matunit1 = 10
      matunit2 = 15

      open(unit=matunit2,file=matname2)
      write(matunit2,1000) time,meqn,ngrids
 1000 format(e30.20,'    time', /,
     &      i5,'                 meqn'/,
     &      i5,'                 ngrids')

      close(matunit2)

      open(unit=matunit1,file=matname1,status='replace')
      close(matunit1)

      end

      subroutine fclaw2d_fort_write_file(matname1,
     &      mx,my,meqn,mbc, xlower,ylower, dx,dy,
     &      q,patch_num,level,blockno,mpirank)

      implicit none

      character*10 matname1
      integer meqn,mbc,mx,my
      integer patch_num, level, blockno, mpirank
      double precision xlower, ylower,dx,dy

      double precision q(1-mbc:mx+mbc,1-mbc:my+mbc,meqn)

      integer matunit1
      integer i,j,mq

      matunit1 = 10
      open(matunit1,file=matname1,position='append');

      call fclaw2d_fort_write_grid_header(matunit1,
     &      mx,my,xlower,ylower, dx,dy,patch_num,level,
     &      blockno,mpirank)


      if (meqn .gt. 5) then
         write(6,'(A,A,A)')
     &         'Warning (fclaw2d_fort_write_grid_header.f) ',
     &         ': meqn > 5; change format statement 120.'
         stop
      endif

      do j = 1,my
         do i = 1,mx
            do mq = 1,meqn
               if (abs(q(i,j,mq)) .lt. 1d-99) then
                  q(i,j,mq) = 0.d0
               endif
            enddo
            write(matunit1,120) (q(i,j,mq),mq=1,meqn)
         enddo
         write(matunit1,*) ' '
      enddo
c     # This statement is checked above (meqn <= 5)
  120 format (5E26.16)

      close(matunit1)

      end


      subroutine fclaw2d_fort_write_grid_header(matunit1,
     &      mx,my,xlower,ylower, dx,dy,patch_num,level,
     &      blockno,mpirank)

      implicit none

      integer matunit1, mx, my
      integer patch_num, level, blockno, mpirank
      double precision xlower, ylower,dx,dy


      write(matunit1,1001) patch_num, level, blockno, mpirank, mx, my
 1001 format(i5,'                 grid_number',/,
     &       i5,'                 AMR_level',/,
     &       i5,'                 block_number',/,
     &       i5,'                 mpi_rank',/,
     &       i5,'                 mx',/,
     &       i5,'                 my')


      write(matunit1,1002) xlower,ylower,dx,dy
 1002 format(e24.16,'    xlow', /,
     &       e24.16,'    ylow', /,
     &       e24.16,'    dx', /,
     &       e24.16,'    dy',/)


      end
