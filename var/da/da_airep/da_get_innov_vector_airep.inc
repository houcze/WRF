subroutine da_get_innov_vector_airep( it, grid, ob, iv)

   !-----------------------------------------------------------------------
   ! Purpose: TBD   
   !-----------------------------------------------------------------------

   implicit none

   integer,          intent(in)    :: it       ! External iteration.
   type(domain),     intent(in)    :: grid     ! first guess state.
   type(y_type),     intent(inout) :: ob       ! Observation structure.
   type(iv_type),    intent(inout) :: iv       ! O-B structure.

   integer :: n                         ! Loop counter.
   integer :: i  (kms:kme)
   integer :: j  (kms:kme)
   real    :: dx (kms:kme)
   real    :: dxm(kms:kme)  
   real    :: dy (kms:kme)
   real    :: dym(kms:kme)  
   integer :: k                   ! Index dimension.

   real, allocatable :: model_u(:,:)  ! Model value u at ob location.
   real, allocatable :: model_v(:,:)  ! Model value v at ob location.
   real, allocatable :: model_t(:,:)  ! Model value t at ob location.

   real    :: v_h(kms:kme)      ! Model value h at ob hor. location.
   real    :: v_p(kms:kme)      ! Model value p at ob hor. location.

   integer :: itu,ituf,itvv,itvvf,itt,ittf

   if (trace_use_dull) call da_trace_entry("da_get_innov_vector_airep")

   itu   = 0; itvv    = 0; itt  = 0;
   ituf  = 0; itvvf   = 0; ittf = 0;

   allocate (model_u(iv%info(airep)%max_lev,iv%info(airep)%n1:iv%info(airep)%n2))
   allocate (model_v(iv%info(airep)%max_lev,iv%info(airep)%n1:iv%info(airep)%n2))
   allocate (model_t(iv%info(airep)%max_lev,iv%info(airep)%n1:iv%info(airep)%n2))

   model_u(:,:) = 0.0
   model_v(:,:) = 0.0
   model_t(:,:) = 0.0

   do n=iv%info(airep)%n1, iv%info(airep)%n2
      if (iv%info(airep)%levels(n) < 1) cycle

      ! [1.1] Get horizontal interpolation weights:

      if (position_lev_dependant) then
         i(:)   = iv%info(airep)%i(:,n)
         j(:)   = iv%info(airep)%j(:,n)
         dx(:)  = iv%info(airep)%dx(:,n)
         dy(:)  = iv%info(airep)%dy(:,n)
         dxm(:) = iv%info(airep)%dxm(:,n)
         dym(:) = iv%info(airep)%dym(:,n)
         do k=kts,kte
            v_h(k) = dym(k)*(dxm(k)*grid%xb%h(i(k),j(k),  k) + dx(k)*grid%xb%h(i(k)+1,j(k),  k)) &
                    + dy(k)*(dxm(k)*grid%xb%h(i(k),j(k)+1,k) + dx(k)*grid%xb%h(i(k)+1,j(k)+1,k))
            v_p(k) = dym(k)*(dxm(k)*grid%xb%p(i(k),j(k),  k) + dx(k)*grid%xb%p(i(k)+1,j(k),  k)) &
                    + dy(k)*(dxm(k)*grid%xb%p(i(k),j(k)+1,k) + dx(k)*grid%xb%p(i(k)+1,j(k)+1,k))
         end do
      else
         i(1)   = iv%info(airep)%i(1,n)
         j(1)   = iv%info(airep)%j(1,n)
         dx(1)  = iv%info(airep)%dx(1,n)
         dy(1)  = iv%info(airep)%dy(1,n)
         dxm(1) = iv%info(airep)%dxm(1,n)
         dym(1) = iv%info(airep)%dym(1,n)

         v_h(kts:kte) = dym(1)*(dxm(1)*grid%xb%h(i(1),j(1),  kts:kte) + dx(1)*grid%xb%h(i(1)+1,j(1),  kts:kte)) &
                       + dy(1)*(dxm(1)*grid%xb%h(i(1),j(1)+1,kts:kte) + dx(1)*grid%xb%h(i(1)+1,j(1)+1,kts:kte))
         v_p(kts:kte) = dym(1)*(dxm(1)*grid%xb%p(i(1),j(1),  kts:kte) + dx(1)*grid%xb%p(i(1)+1,j(1),  kts:kte)) &
                       + dy(1)*(dxm(1)*grid%xb%p(i(1),j(1)+1,kts:kte) + dx(1)*grid%xb%p(i(1)+1,j(1)+1,kts:kte))
      end if

      do k=1, iv%info(airep)%levels(n)
         if (iv%airep(n)%p(k) > 1.0) then
            call da_to_zk (iv%airep(n)%p(k), v_p, v_interp_p, iv%info(airep)%zk(k,n))
         else if (iv%airep(n)%h(k) > 0.0) then
            call da_to_zk (iv%airep(n)%h(k), v_h, v_interp_h, iv%info(airep)%zk(k,n))
         end if
      end do
   end do

   call da_convert_zk (iv%info(airep))

   if (.not. anal_type_verify) then
      do n=iv%info(airep)%n1,iv%info(airep)%n2
         do k=1, iv%info(airep)%levels(n)
            if (iv%info(airep)%zk(k,n) < 0.0) then
               iv%airep(n)%u(k)%qc = missing
               iv%airep(n)%v(k)%qc = missing
               iv%airep(n)%t(k)%qc = missing
            end if
         end do
      end do
   end if

   call da_interp_lin_3d (grid%xb%u, iv%info(airep), model_u)
   call da_interp_lin_3d (grid%xb%v, iv%info(airep), model_v)
   call da_interp_lin_3d (grid%xb%t, iv%info(airep), model_t)

   do n=iv%info(airep)%n1, iv%info(airep)%n2

      !-------------------------------------------------------------------
      ! [2.0] Initialise components of innovation vector:
      !-------------------------------------------------------------------

      do k = 1, iv%info(airep)%levels(n)
         iv % airep(n) % u(k) % inv = 0.0
         iv % airep(n) % v(k) % inv = 0.0
         iv % airep(n) % t(k) % inv = 0.0

         !----------------------------------------------------------------
         ! [3.0] Fast interpolation:
         !----------------------------------------------------------------

         if (ob % airep(n) % u(k) > missing_r .AND. iv % airep(n) % u(k) % qc >= obs_qc_pointer) then
            iv % airep(n) % u(k) % inv = ob % airep(n) % u(k) - model_u(k,n)
         end if

         if (ob % airep(n) % v(k) > missing_r .AND. iv % airep(n) % v(k) % qc >= obs_qc_pointer) then
            iv % airep(n) % v(k) % inv = ob % airep(n) % v(k) - model_v(k,n)
         end if

         if (ob % airep(n) % t(k) > missing_r .AND. iv % airep(n) % t(k) % qc >= obs_qc_pointer) then
            iv % airep(n) % t(k) % inv = ob % airep(n) % t(k) - model_t(k,n)
         end if
      end do
   end do

   !-------------------------------------------------------------------
   ! [5.0] Perform optional maximum error check:
   !-------------------------------------------------------------------

   if (check_max_iv) call da_check_max_iv_airep (iv, it,itu,ituf,itvv,itvvf,itt,ittf)
   
   if (rootproc .and. check_max_iv_print) then
      write(unit = check_max_iv_unit, fmt ='(A,i5,A)') &
         'For outer iteration ',it, &
         ', Total Rejections for Airep follows:'
      write(unit = check_max_iv_unit, fmt = '(/,10(2(A,I6),/))') &
         'Number of failed u-wind observations:     ',ituf, ' on ',itu,   &
         'Number of failed v-wind observations:     ',itvvf,' on ',itvv,   &
         'Number of failed temperature observations:',ittf, ' on ',itt,   &
         'Finally Total Airep rejections ',ituf+itvvf+ittf,' on ', &
         itu +itvv +itt
   end if

   deallocate (model_u)
   deallocate (model_v)
   deallocate (model_t)
   
   if (trace_use_dull) call da_trace_exit("da_get_innov_vector_airep")

end subroutine da_get_innov_vector_airep

