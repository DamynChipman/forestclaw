c     # ------------------------------------------------------------------
c     # MAPC2M_SPHERE
c     # ------------------------------------------------------------------
c     #
c     # Maps a logically rectangular Cartesian grid in [-1,1]x[-1,1] to
c     # either the upper hemisphere (blockno == 0) or the lower hemisphere
c     # (blockno == 1).
c     #
c     # ------------------------------------------------------------------
      subroutine mapc2m_sphere(xc1,yc1,xp,yp,zp)
      implicit none

      double precision xc1,yc1, xp, yp, zp

      double precision x1, y1, d, rp2, xc, yc
      logical multiblock
      integer blockno, get_block

c     # If 'multiblock == .true.', then this map is applied in a multiblock
c     # setting where each mesh is assumed to be n [-1,1]x[-1,1], and an
c     # independent function is used to return the current block number
c     # If used in the non-multiblock case, it is assumed that all data comes
c     # from the domain [-3,1]x[-1,1].
      data multiblock /.true./

      xc = xc1
      yc = yc1

c     # Translate and flip [-3,-1]x[-1,1] into [1,-1]x[-1,1]
      if (.not. multiblock) then
         if (xc .lt. -1) then
            blockno = 1
            xc = -(xc+2)
         else
            blockno = 0
         endif
      else
         blockno = get_block()
      endif


c     # Map xc and yc from ghost cells to interior
      d = max(xc - 1,0.d0) + max(-1 - xc,0.d0)
      x1 = ((1-d)/(1+d))*xc

      d = max(yc - 1,0.d0) + max(-1 - yc,0.d0)
      y1 = ((1-d)/(1+d))*yc


c     # Get circle of radius sqrt(2.d0).  Cluster radial
c     # direction towards boundary
      call mapc2p_circle_sp(x1,y1,xp,yp)

c     # Set z value
      rp2 = xp**2 + yp**2
      if (abs(rp2 - 2.d0) .lt. 1d-10) then
         zp = 0.d0
      else
         zp = sqrt(2.d0 - rp2)
      endif

c     # Values that are outside of [-1,1]x[-1,1] (in ghost cell regions)
c     # to the lower hemisphere
      if (abs(yc) .gt. 1 .or. abs(xc) .gt. 1) then
         zp = -zp
      endif


c     # This maps everything to the unit sphere
      xp = xp/sqrt(2.d0)
      yp = yp/sqrt(2.d0)
      zp = zp/sqrt(2.d0)

c     # Set lower hemisphere
      if (blockno .eq. 1) then
         zp = -zp
      endif

      return
      end
c --------------------------------- MAPC2M -------------------------------


c     # Map single grid to the disk.  Since this disk will be used for the
c     #
      subroutine mapc2p_circle_sp(x1,y1,xp,yp)
      implicit none

      double precision xc,yc,xp,yp, x1,y1
      double precision xi,eta,x,y, minxy,maxxy
      double precision xit, etat, dd

      double precision pi
      common /compi/ pi

      xc = x1
      yc = y1

      if (x1 .lt. -1.d0) then
         xc = -(xc + 2)
      endif

      xi = min(abs(xc),abs(yc))
      eta = max(abs(xc),abs(yc))
      eta = max(eta,1.d-10)

      dd = sin(pi*eta/2.d0)
c      dd = eta*(2-eta)

      xit = (xi/eta)*dd
      etat = dd

      call map_north_sector_sp(xit,etat,x,y)

      minxy = min(abs(x),abs(y))
      maxxy = max(abs(x),abs(y))

      if (abs(xc) .le. abs(yc))  then
c        # In NS sectors
         xp = sign(1.d0,xc)*minxy
         yp = sign(1.d0,yc)*maxxy
      else
c        # In EW sectors
         xp = sign(1.d0,xc)*maxxy
         yp = sign(1.d0,yc)*minxy
      endif


      end

      subroutine map_north_sector_sp(xi,eta,x,y)
      implicit none

      double precision xi,eta,x,y

      x = xi
      y = sqrt(2 - xi**2) - sqrt(2 - eta**2) + eta

      end
