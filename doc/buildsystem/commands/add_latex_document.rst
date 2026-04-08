add_latex_document
==================

This command is provided by :doc:`../modules/UseLatexMk`.

.. cmake:command:: add_latex_document

  Add a custom target that builds a PDF document from a LaTeX source file.

  .. code-block:: cmake

    add_latex_document(
      SOURCE <tex-source>
      [TARGET <target-name>]
      [EXCLUDE_FROM_ALL]
      [REQUIRED]
      [FATHER_TARGET <meta-target>...]
      [RCFILE <latexmkrc>...]
      [INSTALL <destination>]
      [BUILD_ON_INSTALL]
    )

  ``SOURCE``
    Required path to the main LaTeX source file.

  ``TARGET``
    Optional target name. If omitted, a name is derived from the source path.
    A corresponding ``<target>_clean`` target is also created.

  ``EXCLUDE_FROM_ALL``
    Exclude the generated target from the default ``all`` target. This is
    implied when ``FATHER_TARGET`` is specified.

  ``REQUIRED``
    Fail with a fatal error if the document cannot be built because LaTeX or
    ``latexmk`` is unavailable.

  ``FATHER_TARGET``
    Existing meta-targets that should depend on the generated document target.

  ``RCFILE``
    Additional ``latexmkrc`` files that are configured with
    :dune:cmake-command:`configure_file` and loaded after the automatically
    generated rc file. This is an advanced customization mechanism and can also
    affect non-PDF builds.

  ``INSTALL``
    Install destination for the generated PDF document.

  ``BUILD_ON_INSTALL``
    Trigger a build of the document during installation.

  The module also creates a global ``clean_latex`` target that removes LaTeX
  output and auxiliary files.

