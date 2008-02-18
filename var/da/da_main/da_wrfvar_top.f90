module da_wrfvar_top

   !-----------------------------------------------------------------------
   ! Purpose: 
   !-----------------------------------------------------------------------

   use module_configure, only : grid_config_rec_type,model_config_rec, &
      model_to_grid_config_rec, get_config_as_buffer,set_config_as_buffer, &
      initial_config,nl_get_debug_level
   use module_domain, only : domain,alloc_and_configure_domain, head_grid, &
      program_name
   use module_driver_constants, only : max_comms
   use module_symbols_util, only : wrfu_finalize, wrfu_initialize, &
      wrfu_cal_gregorian

   use module_state_description, only : num_moist, num_a_moist, num_g_moist, &
      num_dfi_moist, num_scalar, num_a_scalar, num_g_scalar, num_dfi_scalar, &
      num_fdda3d, num_fdda2d, num_ozmixm, num_aerosolc
   use module_tiles, only : set_tiles

#ifdef DM_PARALLEL
   use module_dm, only : local_communicator, local_communicator_x, &
      local_communicator_y, ntasks_x, ntasks_y, data_order_xyz, mytask, &
      ntasks, data_order_xy,wrf_dm_initialize
   use module_comm_dm, only : halo_radar_xa_w_sub,halo_ssmi_xa_sub, &
      halo_sfc_xa_sub, halo_xa_sub, halo_psichi_uv_adj_sub, halo_bal_eqn_adj_sub, &
      halo_psichi_uv_sub, halo_init_sub, halo_psichi_uv_adj_sub, halo_2d_work_sub
#endif

   ! too many namelist options to list
   use da_control
   use da_define_structures, only : y_type, j_type, iv_type, be_type, &
      xbx_type,da_deallocate_background_errors,da_initialize_cv, &
      da_zero_vp_type,da_allocate_y,da_deallocate_observations, &
      da_deallocate_y
   use da_minimisation, only : da_get_innov_vector,da_minimise_cg, &
      da_write_diagnostics
   use da_obs_io, only : da_write_filtered_obs
   use da_par_util, only : da_system,da_copy_tile_dims,da_copy_dims
   use da_physics, only : da_uvprho_to_w_lin
   use da_reporting, only : message, da_warning, da_error, da_message
   use da_setup_structures, only : da_setup_obs_structures, &
      da_setup_background_errors,da_setup_flow_predictors
   use da_test, only : da_check
   use da_tools_serial, only : da_get_unit, da_free_unit
   use da_tracing, only : da_trace_entry, da_trace_exit, da_trace
   use da_transfer_model, only : da_transfer_xatoanalysis,da_setup_firstguess
   use da_vtox_transforms, only : da_transform_vtox
   use da_wrfvar_io, only : da_med_initialdata_input, da_med_initialdata_output
   use da_tools, only : da_set_randomcv

   use da_wrf_interfaces

   implicit none

   integer :: loop, levels_to_process

   type (domain) , pointer :: keep_grid, grid_ptr, null_domain
   type (grid_config_rec_type), save :: config_flags
   integer                 :: number_at_same_level
   integer                 :: time_step_begin_restart

   integer :: domain_id , fid , oid , idum1 , idum2

#ifdef DM_PARALLEL
   integer                 :: nbytes
   integer, parameter      :: configbuflen = 4* CONFIG_BUF_LEN
   integer                 :: configbuf( configbuflen )
#endif

   character (len=80)      :: rstname

contains

#include "da_wrfvar_init1.inc"
#include "da_wrfvar_init2.inc"
#include "da_wrfvar_run.inc"
#include "da_wrfvar_interface.inc"
#include "da_wrfvar_finalize.inc"
#include "da_solve.inc"

end module da_wrfvar_top