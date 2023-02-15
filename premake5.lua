-- premake5.lua
workspace "1stRayTracing"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }
   startproject "1stRayTracing"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include "Walnut/WalnutExternal.lua"

include "1stRayTracing"