# searches for grid_dim parameter

# asks for grid-dim and pass to ALL_PKG_CPPFLAGS
AC_DEFUN([DUNE_GRID_DIMENSION],[

# default is disabled 
  AC_ARG_WITH(grid_dim,
              AS_HELP_STRING([--with-grid-dim=1|2|3],
                             [dimension of grid; mainly used to determine grid
                              dimension during compilation (default=disabled)]),
                              , with_grid_dim=0)
  AC_ARG_WITH(world_dim,
              AS_HELP_STRING([-with-world-dim=1|2|3],
                             [dimension of world; mainly used to determine grid
                              dimension during compilation (default=grid-dim)]),
              , with_world_dim=0)

  AC_ARG_WITH(grid_type,
            AS_HELP_STRING([--with-grid-type= ALBERTAGRID | ALUGRID_CUBE | ALUGRID_SIMPLEX | ALUGRID_CONFORM | ONEDGRID | SGRID | UGGRID | YASPGRID],
          [only enabled if grid-dim is given, sets type of grid (default=YASPGRID)]),, with_grid_type=YASPGRID)
          
AS_IF([test "x$with_grid_dim" != "x0"],[
  
  AC_MSG_CHECKING([grid-dim and grid-type])
  # make sure that given grid dim is valid 
  AS_IF([test "x$with_grid_dim" != "x1" -a "x$with_grid_dim" != "x2" -a "x$with_grid_dim" != "x3"],[
    AC_MSG_ERROR([Delivered grid-dim = $with_grid_dim is not valid. Valid are only 1, 2, or 3 .])
  ])

  # make sure that given grid type is valid 
  AS_IF([test "x$with_grid_type" != "xALBERTAGRID" -a
              "x$with_grid_type" != "xALUGRID_CUBE" -a
              "x$with_grid_type" != "xALUGRID_SIMPLEX" -a
              "x$with_grid_type" != "xALUGRID_CONFORM" -a
              "x$with_grid_type" != "xONEDGRID" -a
              "x$with_grid_type" != "xSGRID" -a
              "x$with_grid_type" != "xUGGRID" -a
              "x$with_grid_type" != "xYASPGRID"],[
    AC_MSG_ERROR([Delivered grid-type = $with_grid_type is not valid. Run ./configure --help for valid values.])
  ])

  variable_griddim="$``(``GRIDDIM``)``"
  variable_worlddim="$``(``WORLDDIM``)``"
  griddim_cppflags="-DGRIDDIM=$``(``GRIDDIM``)`` -DWORLDDIM=$``(``WORLDDIM``)`` -D$``(``GRIDTYPE``)``"
  AC_SUBST(GRIDDIM, $with_grid_dim)
  AS_IF([test "x$with_world_dim" = "x0"],[
    AC_SUBST(WORLDDIM, $variable_griddim)
  ],[
    AC_SUBST(WORLDDIM, $with_world_dim)
  ])
  AC_SUBST(GRIDDIMGRID, $variable_griddim )
  AC_SUBST(GRIDDIMWORLD, $variable_worlddim )
  AC_SUBST(GRIDTYPE, $with_grid_type )
  AC_SUBST(GRIDDIM_CPPFLAGS, $griddim_cppflags)
  DUNE_PKG_CPPFLAGS="$DUNE_PKG_CPPFLAGS $griddim_cppflags"
  AS_IF([test "x$with_world_dim" = "x0"],[
    AC_MSG_RESULT([yes (GRIDDIM=$GRIDDIM, WORLDDIM=GRIDDIM and GRIDTYPE=$GRIDTYPE)])
  ].[
    AC_MSG_RESULT([yes (GRIDDIM=$GRIDDIM, WORLDDIM=$WORLDDIM and GRIDTYPE=$GRIDTYPE)])
])
])
])
