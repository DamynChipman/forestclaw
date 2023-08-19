c> @file
c> Clawpatch utilities

c     ---------------------------------------------------------------
c>    @brief Determines the tranformation from a coarse patch to it's
c>    half-sized neighbor's coordinate system
c>
c>    @param[in]  the pointer to the fclaw2d_patch_transform_data struct
c>    @param[out] a the 2x2 tranform matrix for the i,j indexes of a patch
c>    @param[out] f the transform vector
c     ---------------------------------------------------------------
      subroutine fclaw3d_clawpatch_build_transform(transform_ptr,a,f)
      implicit none

      integer a(2,2)
      integer*8 transform_ptr
      integer f(2)
      integer mi(4),mj(4)
      integer i1,j1

c     # Assume index mapping fclaw2d_transform_face_half has the
c     # the form
c     #
c     #       T(ic,jc) = A*(ic,jc) + F = (iff,jff)
c     #
c     # where (ic,jc) is the coarse grid index, and (iff,jff)
c     # is the first fine grid index.
c     #
c     # We can recover coefficients A(2,2) with the following
c     # calls to T.

      i1 = 0
      j1 = 0
      call fclaw3d_clawpatch_transform_face_half(i1,j1,mi,mj,
     &      transform_ptr)
      f(1) = mi(1)
      f(2) = mj(1)

      i1 = 1
      j1 = 0
      call fclaw3d_clawpatch_transform_face_half(i1,j1,mi,mj,
     &      transform_ptr)
      a(1,1) = mi(1) - f(1)
      a(2,1) = mj(1) - f(2)

      i1 = 0
      j1 = 1
      call fclaw3d_clawpatch_transform_face_half(i1,j1,mi,mj,
     &      transform_ptr)
      a(1,2) = mi(1) - f(1)
      a(2,2) = mj(1) - f(2)

      end

      subroutine fclaw3d_clawpatch_build_transform_same(transform_ptr,a
     &      ,f)
      implicit none

      integer a(2,2)
      integer*8 transform_ptr
      integer f(2)
      integer mi(4),mj(4)
      integer i1,j1

      i1 = 0
      j1 = 0
      call fclaw3d_clawpatch_transform_face(i1,j1,mi,mj,
     &      transform_ptr)
      f(1) = mi(1)
      f(2) = mj(1)

      i1 = 1
      j1 = 0
      call fclaw3d_clawpatch_transform_face(i1,j1,mi,mj,
     &      transform_ptr)
      a(1,1) = mi(1) - f(1)
      a(2,1) = mj(1) - f(2)

      i1 = 0
      j1 = 1
      call fclaw3d_clawpatch_transform_face(i1,j1,mi,mj,
     &      transform_ptr)
      a(1,2) = mi(1) - f(1)
      a(2,2) = mj(1) - f(2)

      end
c --------------------------------------------------------------------
c> @brief checks if the index is valid for averaging
c>
c> @param[in] i, j the idnex to check
c> @param[in] mx, my the number of cells in the x and y directions
c> @return true if the index is valid
c --------------------------------------------------------------------
      logical function fclaw3d_clawpatch_is_valid_average(i,j,k,
     1                                                mx,my,mz)
      implicit none

      integer i,j,k,mx,my,mz
      logical i1, j1, k1

      i1 = 1 .le. i .and. i .le. mx
      j1 = 1 .le. j .and. j .le. my
      k1 = 1 .le. k .and. k .le. mz

      fclaw3d_clawpatch_is_valid_average = i1 .and. j1 .and. k1

      end

