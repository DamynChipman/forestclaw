subroutine latlong_compute_area(mx,my,mbc,dx,dy, & 
    xlower, ylower, blockno, maxlevel, level, area)
    IMPLICIT NONE

    INTEGER mx,my,mbc,blockno, maxlevel, level
    DOUBLE PRECISION dx,dy, xlower, ylower
    DOUBLE PRECISION area(-mbc:mx+mbc+1,-mbc:my+mbc+1)

    double precision pi, pi2
    common /compi/ pi, pi2

    integer exact_metric
    common /metric_comm/ exact_metric

    double precision r_latlong, phi0, phi1
    common /latlong_comm/ r_latlong, phi0, phi1

    integer i,j, rfactor, N
    double precision dth, dphi
    double precision xii,yjj,xlow,ylow,area_sum
    double precision phi, theta, as, b1, b2, h
    double precision dxf, dyf
    integer ii, jj

!!  # Refinement factor    
    rfactor = 2**(maxlevel-level)

!!  Number of cells that go around latlong   
!!  For conservation, we use finest level to estimate the area
    N = mx*2**level*rfactor

    dth = (2*pi/N)
    dphi = pi/N

    dxf = dx/rfactor
    dyf = dy/rfactor

    do j = -mbc,my+mbc+1
        do i = -mbc,mx+mbc+1
            xlow = xlower + (i-1)*dx
            ylow = ylower + (j-1)*dy
            area_sum = 0
            do ii = 0,rfactor-1
                do jj = 0,rfactor-1
                    xii = xlow + ii*dxf
                    yjj = ylow + jj*dyf
                    call map_comp2latlong(xii,yjj,theta,phi)                        
                    if (exact_metric .eq. 1) then
                        as = r_latlong**2*(sin(phi+dphi) - sin(phi))*dth
                    else
                        b1 = 2*r_latlong*cos(phi)*sin(dth/2)
                        b2 = 2*r_latlong*cos(phi+dphi)*sin(dth/2)
                        h = 2*r_latlong*sin(dphi/2)
                        as = h*(b1 + b2)/2  !! Trapezoidal rule
                    endif
                    area_sum = area_sum + as
                end do
            end do
            area(i,j) = area_sum
        end do
    end do
end


SUBROUTINE latlong_compute_tangents(mx,my,mbc, dx,dy, level, & 
    xd,yd,zd, xtangents,ytangents,edge_lengths)
    IMPLICIT NONE

    INTEGER mx,my,mbc, level
    double precision dx,dy

    DOUBLE PRECISION xd(-mbc:mx+mbc+2,-mbc:my+mbc+2)
    DOUBLE PRECISION yd(-mbc:mx+mbc+2,-mbc:my+mbc+2)
    DOUBLE PRECISION zd(-mbc:mx+mbc+2,-mbc:my+mbc+2)

    DOUBLE PRECISION xtangents(-mbc:mx+mbc+2,-mbc:my+mbc+2,3)
    DOUBLE PRECISION ytangents(-mbc:mx+mbc+2,-mbc:my+mbc+2,3)
    DOUBLE PRECISION edge_lengths(-mbc:mx+mbc+2,-mbc:my+mbc+2,2)

    DOUBLE PRECISION pi, pi2
    COMMON /compi/ pi, pi2

    INTEGER exact_metric
    COMMON /metric_comm/ exact_metric

    double precision r_latlong, phi0, phi1
    common /latlong_comm/ r_latlong, phi0, phi1

    INTEGER i,j,m
    DOUBLE PRECISION taup(3),tlen

    INTEGER N
    DOUBLE PRECISION hx, hy, dz, dth

    dth = 2*pi*dx
    dz = dy
    if (exact_metric .eq. 1) then
        hx = r_latlong*dth         
    else
        hx = 2*r_latlong*sin(dth/2)
    endif
    hy = dz

!!  # Get x-face tangents
    DO j = -mbc,my+mbc+1
        DO i = -mbc,mx+mbc+2

            taup(1) = xd(i,j+1) - xd(i,j)
            taup(2) = yd(i,j+1) - yd(i,j)
            taup(3) = zd(i,j+1) - zd(i,j)
            tlen = sqrt(taup(1)**2 + taup(2)**2 + taup(3)**2)

            DO m = 1,3
               xtangents(i,j,m) = taup(m)/tlen
            ENDDO
            edge_lengths(i,j,1) = hy
        ENDDO
    ENDDO

    DO j = -mbc,my+mbc+2
        DO i = -mbc,mx+mbc+1
!!          # Now do y-faces
            taup(1) = xd(i+1,j) - xd(i,j)
            taup(2) = yd(i+1,j) - yd(i,j)
            taup(3) = zd(i+1,j) - zd(i,j)
            tlen = sqrt(taup(1)**2 + taup(2)**2 + taup(3)**2)

            DO m = 1,3
               ytangents(i,j,m) = taup(m)/tlen
            ENDDO
            edge_lengths(i,j,2) = hx
        ENDDO
    ENDDO

END SUBROUTINE  latlong_compute_tangents


SUBROUTINE latlong_compute_normals(mx,my,mbc, &
    xp,yp,zp,xd,yd,zd,xnormals,ynormals)
    IMPLICIT NONE

    INTEGER mx,my,mbc

    DOUBLE PRECISION xp(-mbc:mx+mbc+1,-mbc:my+mbc+1)
    DOUBLE PRECISION yp(-mbc:mx+mbc+1,-mbc:my+mbc+1)
    DOUBLE PRECISION zp(-mbc:mx+mbc+1,-mbc:my+mbc+1)

    DOUBLE PRECISION xd(-mbc:mx+mbc+2,-mbc:my+mbc+2)
    DOUBLE PRECISION yd(-mbc:mx+mbc+2,-mbc:my+mbc+2)
    DOUBLE PRECISION zd(-mbc:mx+mbc+2,-mbc:my+mbc+2)

    DOUBLE precision xnormals(-mbc:mx+mbc+2,-mbc:my+mbc+2,3)
    DOUBLE precision ynormals(-mbc:mx+mbc+2,-mbc:my+mbc+2,3)

    INTEGER i,j,m, ibc, jbc
    DOUBLE PRECISION taud(3),taup(3),nv(3), sp, xv(3)

!!  # Compute normals at all interior edges.


!!  # Get x-face normals
    DO j =  -mbc,my+mbc+1
        DO i = 1-mbc,mx+mbc+1

            taud(1) = xp(i,j) - xp(i-1,j)
            taud(2) = yp(i,j) - yp(i-1,j)
            taud(3) = zp(i,j) - zp(i-1,j)

            taup(1) = xd(i,j+1) - xd(i,j)
            taup(2) = yd(i,j+1) - yd(i,j)
            taup(3) = zd(i,j+1) - zd(i,j)

            CALL get_normal(taup,taud,nv,sp)

            DO m = 1,3
               xnormals(i,j,m) = nv(m)
            ENDDO
        ENDDO
    ENDDO

    DO j = 1-mbc,my+mbc+1
        DO i =  -mbc,mx+mbc+1
!!          # Now do y-faces
            taud(1) = xp(i,j) - xp(i,j-1)
            taud(2) = yp(i,j) - yp(i,j-1)
            taud(3) = zp(i,j) - zp(i,j-1)

            taup(1) = xd(i+1,j) - xd(i,j)
            taup(2) = yd(i+1,j) - yd(i,j)
            taup(3) = zd(i+1,j) - zd(i,j)

            call get_normal(taup,taud,nv,sp)

!!          # nv has unit length
            DO m = 1,3
               ynormals(i,j,m) = nv(m)
            ENDDO
        ENDDO
    ENDDO

END SUBROUTINE latlong_compute_normals



