      subroutine tag4refinement(mx,my,mbc,
     &      meqn, xlower,ylower,dx,dy,blockno,
     &      q, tag_threshold, init_flag,tag_patch)
      implicit none

      integer mx,my, mbc, meqn, tag_patch, init_flag
      integer blockno
      double precision xlower, ylower, dx, dy
      double precision tag_threshold
      double precision q(1-mbc:mx+mbc,1-mbc:my+mbc,meqn)

      integer i,j, mq
      double precision qmin, qmax, xc

      tag_patch = 0

c     # Refine based only on first variable in system.
      mq = 1
      qmin = q(1,1,mq)
      qmax = q(1,1,mq)
      do j = 1,my
         do i = 1,mx
c            qmin = min(q(i,j,mq),qmin)
c            qmax = max(q(i,j,mq),qmax)
c            if (qmax - qmin .gt. tag_threshold) then
c               tag_patch = 1
c               return
c            endif
            xc = xlower + (i-0.5)*dx
            if (xc .gt. 0.5d0) then
               tag_patch = 1
               return
            endif

         enddo
      enddo

      end
