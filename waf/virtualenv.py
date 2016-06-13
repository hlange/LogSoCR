#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set expandtab:ts=4:sw=4:setfiletype python
import os, sys
import subprocess
from waflib.TaskGen import taskgen_method
from waflib.Errors import ConfigurationError
from waflib import Context
from waflib import Task
from waflib import TaskGen
from waflib import Utils
from common import conf

def options(self):
  pass

"""
    # Makes existing virtual env relocatable (for installation?)
    self.cmd_and_log(
        [self.env.VIRTUALENV, "--relocatable", self.env.VENV_PATH],
        output=Context.BOTH,
        cwd=self.bldnode.abspath()
    )
"""
def python_get(self, name):
    self.start_msg("Install %s into virtualenv" % (name))
    self.cmd_and_log(
        [Utils.subst_vars('${VPIP}',self.env), "install", name],
        output=Context.BOTH,
        cwd=self.env.VENV_PATH
    )
    self.end_msg("ok")
conf(python_get)

@Task.always_run
class venv_link_task(Task.Task):
  """Link a Python source directory into the site-packages dir of the venv"""
  name = 'venv_link'
  color = 'BLUE'
  before = []
  quiet = True

  def __str__(self):
      return "venv: %s -> virtualenv\n" % (os.path.basename(self.generator.path.get_bld().abspath()))

  def run(self):
      sdirnode = self.generator.path.get_bld()
      sdirnode.mkdir()
      for snode in self.inputs:
          if not os.path.exists(snode.abspath()):
              dnode = snode.get_bld()
              if not os.path.isdir(dnode.parent.abspath()):
                  dnode.parent.mkdir()
              if not os.path.exists(dnode.abspath()):
                  os.symlink(os.path.relpath(snode.abspath(), os.path.join(dnode.abspath(), "..")), dnode.abspath())
      initnode = sdirnode.find_or_declare('__init__.py')
      if not os.path.exists(initnode.abspath()):
          initnode.write("")
      snode = sdirnode.abspath()
      dnode = os.path.join(self.env["VENV_PATH"], "lib", ("python%s" % self.env.PYTHON_VERSION), "site-packages", os.path.basename(snode))
      if not os.path.exists(dnode):
          os.symlink(os.path.relpath(snode, os.path.join(dnode, "..")), dnode)
      return 0

@TaskGen.before('process_source', 'process_rule')
@TaskGen.feature('venv_package')
def venv_package(self):
    initnode = self.path.find_or_declare('__init__.py')
    if not os.path.exists(initnode.abspath()):
        initnode.write("")
    srclist = [initnode]
    for src in Utils.to_list(getattr(self, "pysource", [])):
        if isinstance(src, str):
            snode = self.path.find_node(src)
        else:
            snode = src
        if not initnode in srclist:
            srclist.append(snode)
    self.env["VENV_PATH"] = os.path.join(self.bld.bldnode.abspath(), ".conf_check_venv")
    snode = self.path.abspath()
    dnode = self.bld.bldnode.find_dir(os.path.join("lib", ("python%s" % self.env.PYTHON_VERSION), "site-packages", os.path.basename(snode)))
    links = self.create_task('venv_link', src=list(srclist), tgt=dnode)

def configure(self):
    try:
        if sys.version_info >= (3, 0):
            self.find_program('virtualenv', var="VIRTUALENV", mandatory=True, okmsg="ok")
        else:
            self.find_program('virtualenv2', var="VIRTUALENV", mandatory=True, okmsg="ok")
    except ConfigurationError as e:
        name    = "virtualenv"
        version = "trunk"
        self.dep_fetch(
          name    = name,
          version = version,
          git_url = "https://github.com/pypa/virtualenv.git",
        )
        self.find_program('virtualenv.py', var="VIRTUALENV", mandatory=True, okmsg="ok", path_list=[self.dep_path(name, version)])
    self.start_msg("Create python virtualenv")
    self.env["VENV_PATH"] = os.path.join(self.bldnode.abspath(), ".conf_check_venv")
    self.cmd_and_log(
        [Utils.subst_vars('${VIRTUALENV}',self.env), "-p", sys.executable, self.env.VENV_PATH],
        output=Context.BOTH,
        cwd=self.bldnode.abspath()
    )
    self.end_msg("ok")
    self.find_program('python', var="VPYTHON", mandatory=True, okmsg="ok", path_list=[os.path.join(self.env.VENV_PATH, "bin")])
    self.find_program('pip', var="VPIP", mandatory=True, okmsg="ok", path_list=[os.path.join(self.env.VENV_PATH, "bin")])

