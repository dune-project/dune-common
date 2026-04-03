LATEXMK_PARANOID
================

This variable is documented in :doc:`../modules/UseLatexMk`.

.. cmake:variable:: LATEXMK_PARANOID

  Re-enable TeX's stricter output-directory security behavior.

  By default, ``UseLatexMk.cmake`` relaxes that behavior by setting
  ``openout_any=a`` for the ``latexmk`` invocation, because some TeX tools
  refuse to write output files outside the current working directory or one of
  its subdirectories.

  This matters when ``latexmk`` is asked to place generated files in absolute
  output directories or in directories above the current working directory. In
  those cases, helper tools such as ``makeindex`` or ``bibtex`` may otherwise
  refuse to write their outputs as part of TeX's security model.

  Setting ``LATEXMK_PARANOID`` to ``TRUE`` restores the stricter default TeX
  behavior. This may break some document builds depending on the TeX toolchain
  and auxiliary tools in use.

