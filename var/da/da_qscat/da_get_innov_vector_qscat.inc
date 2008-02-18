subroutine da_get_innov_vector_qscat (it, grid, ob, iv)

   !-----------------------------------------------------------------------
   ! Purpose: TBD
   !-----------------------------------------------------------------------

   implicit none

   integer,          intent(in)    :: it      ! External iteration.
   type(domain),     intent(in)    :: grid    ! first guess state.
   type(y_type),     intent(inout) :: ob      ! Observation structure.
   type(iv_type),    intent(inout) :: iv      ! O-B structure.

   integer :: n        ! Loop counter.
   integer :: i, j, k  ! Index dimension.
   real    :: dx, dxm  ! Interpolation weights.
   real    :: dy, dym  ! Interpolation weights.

   real, allocatable :: model_u(:,:)  ! Model value u at ob location.
   real, allocatable :: model_v(:,:)  ! Model value v at ob location.

   real    :: v_h(kms:kme)      ! Model value h at ob hor. location.

   integer  :: itu, itvv, ituf, itvvf
   
   if (trace_use_dull) call da_trace_entry("da_get_innov_vector_qscat")

   itu = 0 ; itvv = 0 ;  ituf = 0 ; itvvf = 0

   allocate (model_u(iv%info(qscat)%max_lev,iv%info(qscat)%n1:iv%info(qscat)%n2))
   allocate (model_v(iv%info(qscat)%max_lev,iv%info(qscat)%n1:iv%info(qscat)%n2))

   do n=iv%info(qscat)%n1,iv%info(qscat)%n2

      ! [1.1] Get horizontal interpolation weights:

      i   = iv%info(qscat)%i(1,n)
      j   = iv%info(qscat)%j(1,n)
      dx  = iv%info(qscat)%dx(1,n)
      dy  = iv%info(qscat)%dy(1,n)
      dxm = iv%info(qscat)%dxm(1,n)
      dym = iv%info(qscat)%dym(1,n)

      do k=kts,kte
         v_h(k) = dym*(dxm*grid%xb%h(i,j,k)+dx*grid%xb%h(i+1,j,k)) + dy*(dxm*grid%xb%h(i,j+1,k)+dx*grid%xb%h(i+1,j+1,k))
      end do

      if (iv % qscat(n) % h > missing_r) then
         call da_to_zk(iv % qscat(n) % h, v_h, v_interp_h, iv%info(qscat)%zk(1,n))
         if (iv%info(qscat)%zk(1,n) < 1.0) then
            iv%info(qscat)%zk(1,n) = 1.0
         end if
      end if
   end do

   call da_convert_zk (iv%info(qscat))

   if (.not. anal_type_verify) then
      do n=iv%info(qscat)%n1,iv%info(qscat)%n2
         if (iv%info(qscat)%zk(1,n) < 0.0) then
            iv%qscat(n)%u%qc = missing
            iv%qscat(n)%v%qc = missing
         end if
      end do
   end if

   call da_interp_lin_3d (grid%xb%u, iv%info(qscat), model_u)
   call da_interp_lin_3d (grid%xb%v, iv%info(qscat), model_v)

   do n=iv%info(qscat)%n1,iv%info(qscat)%n2

      !------------------------------------------------------------------------
      ! [2.0] Initialise components of innovation vector:
      !------------------------------------------------------------------------

      !------------------------------------------------------------------------
      ! [3.0] Fast interpolation:
      !------------------------------------------------------------------------

      if (ob % qscat(n) % u > missing_r .AND. &
          iv % qscat(n) % u % qc >= obs_qc_pointer) then
         iv % qscat(n) % u % inv = ob % qscat(n) % u - model_u(1,n)
      end if

      if (ob % qscat(n) % v > missing_r .AND. &
          iv % qscat(n) % v % qc >= obs_qc_pointer) then
         iv % qscat(n) % v % inv = ob % qscat(n) % v - model_v(1,n)
      end if
   end do

   !------------------------------------------------------------------------
   ! [5.0] Perform optional maximum error check:
   !------------------------------------------------------------------------

   if (check_max_iv) call da_check_max_iv_qscat(iv, it, itu, itvv, ituf,itvvf)

   if (rootproc .and. check_max_iv_print) then
      write(unit = check_max_iv_unit, fmt ='(A,i5,A)')'For outer iteration ',it,&
         ', Total Rejections for qscat follows:'
      write(unit = check_max_iv_unit, fmt = '(/,4(2(A,I6),/))') &
         'Number of failed u-wind observations:     ',ituf, ' on ',itu,   &
         'Number of failed v-wind observations:     ',itvvf,' on ',itvv
   end if 
   
   if (trace_use_dull) call da_trace_exit("da_get_innov_vector_qscat")

end subroutine da_get_innov_vector_qscat

