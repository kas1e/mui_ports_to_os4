-- Part of LibMaker package
-- libgen.lua
-- generates complete system library template
-- for AmigaOS 3.x
-- by Grzegorz Kraszewski 2014

require('io')
require('string')
require('base')

-- Declarations.

generator = 'LibMaker 0.11'
generate_autodocs = true
indent_sequence = '\t'
regnames = { "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "a0", "a1", "a2", "a3" }

--[[ 

These two tables contain replacement values to placeholders. The table 'templates' contains
strings to be replaced for simple placeholders (${}). The table 'multiline' contains functions,
which are called for multiline placeholders ($${}). Multiline macros keep indentation inserted
before the placeholder.

]]

templates = {}
multiline = {}


--[[

LibMaker calls the function libgen() (defined at the end of code) with a
multilevel table as an argument. Let's name the table 'spec', as in the code.
Then the table looks like this:

spec['scriptdir']                                            name of subdir in "PROGDIR:scripts/" where the script resides, string
spec['instdir']                                              target dir for compiled binary, for "make install", string
spec['incpath']                                              system include path for the main include file, string
spec['sep_functions']                                        one file per function, or all functions in a single file, boolean
spec['sep_methods']                                          one file per class method, or all methods in a single file, boolean
spec['name']                                                 library name, string
spec['ver']                                                  major version, number
spec['rev']                                                  minor version, number
spec['date']                                                 library date DD.MM.YYYY, string
spec['copyright']                                            copyright text added to $VER:, string
spec['basename']                                             library base name, string
spec['altivec']                                              AltiVec usage, boolean
spec['boopsi']                                               generate BOOPSI class, boolean
spec['mui']                                                  generate MUI class, boolean
spec['functions']                                            table of functions, indexed with integers starting from 1
spec['functions'][1]                                         function definition, table
spec['functions'][1]['name']                                 function name, string
spec['functions'][1]['type']                                 return type, string
spec['functions'][1]['args']                                 function arguments, a table indexed with integers starting from 1
spec['functions'][1]['args'][1]                              argument definition, table
spec['functions'][1]['args'][1]['name']                      argument formal name, string
spec['functions'][1]['args'][1]['type']                      argument type, string
spec['functions'][1]['args'][1]['m68kreg']                   M68k CPU register of the argument, integer (0-7 for D0-D7, 8-11 for A0-A3)
spec['functions'][1]['args'][2]
...
spec['functions'][2]
...

If a library is also a BOOPSI/MUI class (spec['mui'] or spec['boopsi'] is true):

spec['class']
spec['class']['super']                                       superclass name/ID, string
spec['class']['attributes']                                  table of class attributes, indexed from 1
spec['class']['attributes'][1]                               attribute definition, table
spec['class']['attributes'][1]['name']                       attribute name, string
spec['class']['attributes'][1]['id']                         attribute ifentifier, hexadecimal string
spec['class']['attributes'][1]['init']                       usable on init, boolean
spec['class']['attributes'][1]['set']                        settable, boolean
spec['class']['attributes'][1]['get']                        gettable, boolean
spec['class']['attributes'][2]
...
spec['class']['methods']                                     table of class methods, indexed from 1
spec['class']['methods'][1]                                  method definition, table
spec['class']['methods'][1]['name']                          method name, string
spec['class']['methods'][1]['id']                            method identifier, hexadecimal string
spec['class']['methods'][1]['message']                       name of method message structure, string
spec['class']['methods'][2]
...

If a function has no arguments, subtable 'args' still exists, just has no items. Similarly 'attributes' and
'methods' tables for a class.

]]

--------------------------------------------------------------------------------
-- Generator helper functions.
--------------------------------------------------------------------------------

indent_level = 0;



function indent_in()
	indent_level = indent_level + 1
end



function indent_out()
	indent_level = indent_level - 1
end



function indent(fh)
	if (indent_level > 0) then
		for i = 1, indent_level do
			fh:write(indent_sequence)
		end
	elseif (indent_level < 0) then
		error('Negative indent level.', 2)
	end
end



function iwrite(fh, str)
	indent(fh)
	fh:write(str)
end



function autodoc_template(spec, fspec, kind)
	local stars
	local fullname
	local prefix
	local s

	fullname = spec.name .. '/' .. fspec.name
	stars = string.rep('*', 68 - #fullname)
	s = '/****** ' .. fullname .. ' ' .. stars .. '\n'

	if (kind == 'function') then
		s = s .. '*\n* NAME\n*   ' .. fspec.name .. ' -- {short} (V' .. spec.ver .. ')\n*\n'
		s = s .. '* SYNOPSIS\n*   ' .. fspec.type .. ' ' .. fspec.name

		if (#fspec.args == 0) then s = s .. ('(VOID') end

		for arg = 1, #fspec.args do
			if (arg == 1) then prefix = '(' else prefix = ', ' end
			s = s .. prefix .. fspec.args[arg].type .. ' ' .. fspec.args[arg].name
		end

		s = s .. ')\n*\n* FUNCTION\n*\n* INPUTS\n*\n* RESULT\n*\n* SEE ALSO\n*\n'
	elseif (kind == 'background') then
		s = s .. '*\n* DESCRIPTION\n*\n* HISTORY\n*\n'
	end

	stars = string.rep('*', 77)
	s = s .. stars .. '\n*\n*/'
	return s
end

------------------------------------------------------------------------------------------------

function func_filename(funcname, extension)
	return 'f_' .. string.lower(funcname) .. extension
end

function meth_filename(methname, extension)
	return 'm_' .. string.lower(string.gsub(methname, '^(.+_)', '')) .. extension
end

-- Generators for multiline macros
------------------------------------------------------------------------------------------------


multiline['EXTRABASEFIELDS'] = function(indent)
	local s = ''
	return s
end

------------------------------------------------------------------------------------------------

multiline['DEPENDENCIES'] = function(indent)
	local s = ''
	for func = 1, #spec.functions do
		local name, prefix
		if func == 1 then prefix = '' else prefix = '\n' end
		name = spec.functions[func].name
		s = s .. prefix .. func_filename(name, '.o') .. ': ' .. func_filename(name, '.c') .. ' library.h'
	end
	return s
end

------------------------------------------------------------------------------------------------

multiline['FUNCOBJECTS'] = function(indent)
	if not spec.sep_functions then return '' end
	local s = 'FUNCOBJS = '
	for func = 1, #spec.functions do
		if func > 1 then s = s .. ' \\\n ' end
		s = s .. func_filename(spec.functions[func].name, '.o')
	end
	return s .. '\n'
end

------------------------------------------------------------------------------------------------

multiline['METHODOBJECTS'] = function(indent)
	if not spec.doclass then return '' end
	if not spec.sep_methods then return '' end
	local s = 'METHOBJS = m_new.o \\\n m_dispose.o'
	if spec.class.settable_attrs > 0 then s = s .. ' \\\n m_set.o' end
	if spec.class.gettable_attrs > 0 then s = s .. ' \\\n m_get.o' end
		for meth = 1, #spec.class.methods do
		s = s .. ' \\\n '
		s = s .. meth_filename(spec.class.methods[meth].name, '.o')
	end
	return s .. '\n'
end

------------------------------------------------------------------------------------------------

multiline['BACKGROUNDAUTODOC'] = function(indent)
	if generate_autodocs then
		local dummy_fspec
		dummy_fspec = {}
		dummy_fspec.name = 'background'
		return autodoc_template(spec, dummy_fspec, 'background')
	else return '' end
end

------------------------------------------------------------------------------------------------

multiline['EXTRALIBINCLUDES'] = function(indent)
	local s = ''
	local namepart
	local incpath
	incpath = string.gsub(spec.incpath, '/$', '')
	if spec.doclass then s = indent .. '#include <proto/intuition.h>\n' .. indent .. '#include <proto/utility.h>\n' end
	if spec.mui then s = s .. indent .. '#include <proto/muimaster.h>\n' end

	if spec.boopsi then
		namepart = string.match(spec.name, '(.-)%..+')
		s = s .. indent .. '#include <' .. incpath .. '/' .. namepart .. '.h>\n'
	end

	if spec.mui then
		namepart = 'to_do'
		s = s .. indent .. '#include <' .. incpath .. '/' .. namepart .. '_mcc.h>\n'
	end

	return s
end

------------------------------------------------------------------------------------------------

multiline['EXTRALIBBASES'] = function(indent)
	local s = ''
	if spec.doclass then s = indent .. 'struct Library *IntuitionBase;\n' .. indent .. 'struct Library *UtilityBase;\n' end
	if spec.mui then s = s .. indent .. 'struct Library *MUIMasterBase;\n' end
	return s
end

------------------------------------------------------------------------------------------------

multiline['LIBFUNCPROTOS'] = function(indent)
	local s = ''
	for func = 1, #spec.functions do
		fspec = spec.functions[func]
		s = s .. indent .. fspec.type .. ' Lib' .. fspec.name .. '(struct MyLibBase *base'

		for arg = 1, #fspec.args do
			s = s .. ', ' .. fspec.args[arg].type .. ' ' .. fspec.args[arg].name
		end

		s = s .. ');\n'
	end
	return s
end

------------------------------------------------------------------------------------------------

multiline['OPENEXTRALIBS'] = function(indent)
	local s = ''
	if spec.doclass then
		s = indent .. 'if (!(IntuitionBase = OpenLibrary("intuition.library", 50))) return FALSE;\n'
		s = s .. indent .. 'if (!(UtilityBase = OpenLibrary("utility.library", 50))) return FALSE;\n'
	end
	if spec.mui then
		s = s .. indent .. 'if (!(MUIMasterBase = OpenLibrary("muimaster.library", 20))) return FALSE;\n'
	end
	return s
end

------------------------------------------------------------------------------------------------

multiline['CLOSEEXTRALIBS'] = function(indent)
	local s = ''
	if spec.mui then
		s = indent .. 'if (MUIMasterBase) CloseLibrary(MUIMasterBase);\n'
		s = s .. indent .. 'if (UtilityBase) CloseLibrary(UtilityBase);\n'
		s = s .. indent .. 'if (IntuitionBase) CloseLibrary(IntuitionBase);\n'
	elseif spec.boopsi then
		s = indent .. 'if (UtilityBase) CloseLibrary(UtilityBase);\n'
		s = s .. indent .. 'if (IntuitionBase) CloseLibrary(IntuitionBase);\n'
	end
	return s
end

------------------------------------------------------------------------------------------------

multiline['JUMPTABLE'] = function(indent)
	local s = ''

	if #spec.functions > 0 then
		for func = 1, #spec.functions do
			s = s .. indent .. '(APTR)Lib' .. spec.functions[func].name .. ',\n'
		end
	end
	return s
end

------------------------------------------------------------------------------------------------



function template_preparator()
	local s, gets, sets

	spec.doclass = spec.mui or spec.boopsi

	if spec.doclass then
		gets = 0
		sets = 0
		for i = 1, #spec.class.attributes do
			if spec.class.attributes[i].set then sets = sets + 1 end
			if spec.class.attributes[i].get then gets = gets + 1 end
		end
		spec.class.settable_attrs = sets
		spec.class.gettable_attrs = gets
	end

	templates['GENERATOR'] = generator
	templates['LIBNAME'] = spec.name
	templates['LIBVERSION'] = spec.ver
	templates['LIBREVISION'] = spec.rev
	templates['LIBDATE'] = spec.date
	templates['LIBCOPYRIGHT'] = spec.copyright
	templates['DEFINENAME'] = string.gsub(string.upper(string.match(spec.name, '(.+)%..-')), '%-', '_')
	templates['ALTIVECFLAGS'] = ''
	s = string.gsub(spec.instdir, '/$', '')
	s = string.gsub(s, '^(.+):', '/%1/')
	templates['INSTDIR'] = s
	if spec.altivec then templates['ALTIVECFLAGS'] = ' -fvec' else templates['ALTIVECFLAGS'] = '' end
	if spec.doclass then
		templates['SUPERCLASS'] = spec.class.super
	end

	templates['LIBINCLUDE'] = '<' .. string.gsub(spec.incpath, '/$', '') .. '/' .. string.match(spec.name, '(.-)%..+') .. '.h>'

end

------------------------------------------------------------------------------------------------

function multiline_template_replacer(indent, template_name)
	return multiline[template_name](indent)
end

------------------------------------------------------------------------------------------------

function generate_simple(input_name, output_name, overwrite)
	local src, out

	if not overwrite then
		out = io.open(output_name, 'r')
		if out then
			io.close(out)
			return
		end
	end

	out = io.open(output_name, 'w')

	for src in io.lines('PROGDIR:scripts/' .. spec.scriptdir .. '/' .. input_name) do
		if #src == 0 then out:write('\n')
		else
			src = string.gsub(src, '^(%s*)%$%${([%u%d]+)}', multiline_template_replacer)
			src = string.gsub(src, '%${([%u%d]+)}', templates)
			out:write(src)
			if #src > 0 and string.sub(src, -1) ~= '\n' then out:write('\n') end
		end
	end

	io.close(out)
end

------------------------------------------------------------------------------------------------

function gen_library_function(fspec, separated)
	local s = ''
	local proto

	if generate_autodocs then s = '\n' .. autodoc_template(spec, fspec, 'function') .. '\n\n' end
	if separated then s = s .. '#include "library.h"\n\n#include ' .. templates['LIBINCLUDE'] .. '\n\n\n'; end
	proto = '__saveds ' .. fspec.type .. ' Lib' .. fspec.name .. '(struct MyLibBase *base __asm("a6")'

	for arg = 1, #fspec.args do
		proto = proto .. ', ' .. fspec.args[arg].type .. ' ' .. fspec.args[arg].name
		proto = proto .. ' __asm("' .. regnames[fspec.args[arg].m68kreg + 1] .. '")'
	end

	s = s .. proto .. ')\n{\n}\n'
	if not separated then s = s .. '\n\n' end
	return s
end

------------------------------------------------------------------------------------------------

function generate_functions()
	local fh
	local fspec

	if spec.sep_functions then
		local fname
		for func = 1, #spec.functions do
			fspec = spec.functions[func]
			fname = 'f_' .. string.lower(fspec.name) .. '.c'
			fh = io.open(fname, 'r')
			if fh then io.close(fh)
			else
				fh = io.open(fname, 'w')
				fh:write(gen_library_function(spec.functions[func], true))
				io.close(fh)
			end
		end
	else
		fh = io.open('library.c', 'a')
		for func = 1, #spec.functions do
			fh:write(gen_library_function(spec.functions[func], false))
		end
		io.close(fh)
	end
end

------------------------------------------------------------------------------------------------

function gen_custom_method(mspec)
	local s = ''
	local methname = string.match(mspec.name, '^%u+_(.+)')
	if spec.sep_methods then s = '#include "library.h"\n\n'
	else s = 'static ' end
	s = s .. 'IPTR ' .. methname .. '(Class *cl, Object *obj'
	if #mspec.message > 0 then s = s .. ', struct ' .. mspec.message .. ' *msg' end
	s = s .. ')\n{\n\tstruct ObjData *d = INST_DATA(cl, obj);\n\n\treturn 0;\n}\n'
	return s
end

------------------------------------------------------------------------------------------------

function gen_method_new()
	local s = ''
	if spec.sep_methods then s = '#include "library.h"\n\n'
	else s = 'static ' end
	s = s .. 'IPTR New(Class *cl, Object *obj, struct opSet *msg)\n{\n\tIPTR newobj = NULL;\n\n'
	s = s .. '\tobj = (Object*)DoSuperMethodA(cl, obj, (Msg)msg);\n\n\tif (obj)\n\t{\n'
	s = s .. '\t\tstruct ObjData *d = INST_DATA(cl, obj);\n\n\t\t// object initialization here, set \'newobj\' to \'obj\' if success\n\n'
	s = s .. '\t\tnewobj = (IPTR)obj;\n\t\}\n\telse CoerceMethod(cl, obj, OM_DISPOSE);\n\n\treturn newobj;\n}\n'
	return s
end

------------------------------------------------------------------------------------------------

function gen_method_dispose()
	local s = ''
	if spec.sep_methods then s = '#include "library.h"\n\n'
	else s = 'static ' end
	s = s .. 'IPTR Dispose(Class *cl, Object *obj, Msg msg)\n{\n\tstruct ObjData *d = INST_DATA(cl, obj);\n\n'
	s = s .. '\t// object destruction here\n\n\treturn DoSuperMethodA(cl, obj, msg);\n}\n'
	return s
end

------------------------------------------------------------------------------------------------

function gen_method_set()
	local s = ''
	local first = true
	if spec.sep_methods then s = '#include "library.h"\n\n'
	else s = 'static ' end
	s = s .. 'IPTR Set(Class *cl, Object *obj, struct opSet *msg)\n{\n\tstruct ObjData *d = INST_DATA(cl, obj);\n'
	s = s .. '\tstruct TagItem *tag, *tagptr;\n\tLONG tagcnt = 0;\n\n\ttagptr = msg->ops_AttrList;\n\n'
	s = s .. '\twhile ((tag = NextTagItem(&tagptr)))\n\t{\n\t\tswitch (tag->ti_Tag)\n\t\t{'
	for i = 1, #spec.class.attributes do
		if spec.class.attributes[i].set then
			if not first then
				s = s .. '\n'
				first = false
			end
			s = s .. '\n\t\t\tcase ' .. spec.class.attributes[i].name .. ':\n\t\t\t\ttagcnt++;\n\t\t\tbreak;\n'
		end
	end
	s = s .. '\t\t}\n\t}\n\n\ttagcnt += DoSuperMethodA(cl, obj, (Msg)msg);\n\treturn tagcnt;\n}\n'
	return s
end

------------------------------------------------------------------------------------------------

function gen_method_get()
	local s = ''
	local first = true
	if spec.sep_methods then s = '#include "library.h"\n\n'
	else s = 'static ' end
	s = s .. 'IPTR Get(Class *cl, Object *obj, struct opGet *msg)\n{\n\tstruct ObjData *d = INST_DATA(cl, obj);\n'
	s = s .. '\tIPTR result = TRUE;\n\n\tswitch (msg->opg_AttrID)\n\t{'
	for i = 1, #spec.class.attributes do
		if spec.class.attributes[i].get then
			if not first then
				s = s .. '\n'
				first = false
			end
			s = s .. '\n\t\tcase ' .. spec.class.attributes[i].name .. ':\n\t\t\t\n\t\tbreak;\n'
		end
	end
	s = s .. '\n\t\tdefault:\n\t\t\tresult = DoSuperMethodA(cl, obj, (Msg)msg);\n\t\tbreak;\n\t}\n\n\treturn result;\n}\n'
	return s
end

------------------------------------------------------------------------------------------------

function gen_dispatcher()
	local s = ''
	local mspec, methname
	if spec.sep_methods then 
		s = '/* Prototypes of methods */\n\nIPTR New(Class *cl, Object *obj, struct opSet *msg);\n'
		s = s .. 'IPTR Dispose(Class *cl, Object *obj, Msg msg);\n'
		if spec.class.settable_attrs > 0 then s = s .. 'IPTR Set(Class *cl, Object *obj, struct opSet *msg);\n' end
		if spec.class.gettable_attrs > 0 then s = s .. 'IPTR Get(Class *cl, Object *obj, struct opGet *msg);\n' end
		for meth = 1, #spec.class.methods do
			mspec = spec.class.methods[meth]
			methname = string.match(mspec.name, '^%u+_(.+)')
			s = s .. 'IPTR ' .. methname .. '(Class *cl, Object *obj'
			if #mspec.message > 0 then s = s .. ', struct ' .. mspec.message .. ' *msg' end
			s = s .. ');\n'
		end
		s = s .. '\n\n'
	end
	s = s .. 'static IPTR ClassDispatcher(void)\n{\n\tClass *cl = (Class*)REG_A0;\n\tObject *obj = (Object*)REG_A2;\n'
	s = s .. '\tMsg msg = (Msg)REG_A1;\n\n\tswitch(msg->MethodID)\n\t{\n'
	s = s ..'\t\tcase OM_NEW: return New(cl, obj, (struct opSet*)msg);\n'
	s = s ..'\t\tcase OM_DISPOSE: return Dispose(cl, obj, msg);\n'
	if spec.class.settable_attrs > 0 then s = s .. '\t\tcase OM_SET: return Set(cl, obj, (struct opSet*)msg);\n' end
	if spec.class.gettable_attrs > 0 then s = s .. '\t\tcase OM_GET: return Get(cl, obj, (struct opGet*)msg);\n' end
	for meth = 1, #spec.class.methods do
		mspec = spec.class.methods[meth]
		methname = string.match(mspec.name, '^%u+_(.+)')
		s = s .. '\t\tcase ' .. mspec.name .. ': return ' .. methname .. '(cl, obj'
		if #mspec.message > 0 then s = s .. ', (struct ' .. mspec.message .. '*)msg' end
		s = s .. ');\n'
	end
	s = s .. '\t\tdefault: return DoSuperMethodA(cl, obj, msg);\n\t}\n}\n'
	return s
end

------------------------------------------------------------------------------------------------

function gen_method_separate(fname, body)
	local fh
	fh = io.open(fname, 'r')
	if fh then
		io.close(fh)
		return
	end
	fh = io.open(fname, 'w')
	fh:write(body)
	io.close(fh)
end

------------------------------------------------------------------------------------------------

function generate_methods()
	local fh, mspec, mname
	if spec.sep_methods then
		gen_method_separate('m_new.c', gen_method_new())
		gen_method_separate('m_dispose.c', gen_method_dispose())
		if spec.class.settable_attrs > 0 then gen_method_separate('m_set.c', gen_method_set()) end
		if spec.class.gettable_attrs > 0 then gen_method_separate('m_get.c', gen_method_get()) end
		for meth = 1, #spec.class.methods do
			mspec = spec.class.methods[meth]
			gen_method_separate(meth_filename(mspec.name, '.c'), gen_custom_method(mspec))
		end
		fh = io.open('library.c', 'a')
		fh:write(gen_dispatcher())
		io.close(fh)
	else
		fh = io.open('library.c', 'a')
		fh:write(gen_method_new() .. '\n\n')
		fh:write(gen_method_dispose() .. '\n\n')
		if spec.class.settable_attrs > 0 then fh:write(gen_method_set() .. '\n\n') end
		if spec.class.gettable_attrs > 0 then fh:write(gen_method_get() .. '\n\n') end
		for meth = 1, #spec.class.methods do
			mspec = spec.class.methods[meth]
			fh:write(gen_custom_method(mspec) .. '\n\n')
		end
		fh:write(gen_dispatcher())
		io.close(fh)
	end
end

------------------------------------------------------------------------------------------------

function geninclude_class_header(subdir)
	local s, path
	namepart = string.match(spec.name, '(.-)%..+')
	incdef = string.gsub(string.upper(namepart), '%-', '_')
	libmaker.makedir('os-include')
	libmaker.makedir('os-include/classes')
	path = 'os-include/classes'

	if subdir then
		incdef = string.upper(subdir) .. '_' .. incdef
		path = path .. '/' .. subdir
		libmaker.makedir(path)
	end

	fh = io.open(path .. '/' .. namepart .. '.h', 'w')
	s = '/*\n    ' .. spec.name .. ' definitions\n\n    Copyright © ' .. spec.copyright
	s = s .. '. All rights reserved.\n*/\n\n'
	s = s .. '#ifndef CLASSES_' .. incdef .. '_H\n#define CLASSES_' .. incdef .. '_H\n\n\n'
	s = s .. '/* attributes */\n\n'

	for attr = 1, #spec.class.attributes do
		local atspec = spec.class.attributes[attr]
		s = s .. '#define ' .. atspec.name .. string.rep(' ', 39 - #atspec.name) .. ' 0x' .. atspec.id .. '\n'
	end

	s = s .. '\n\n/* methods */\n\n'

	for meth = 1, #spec.class.methods do
		local mspec = spec.class.methods[meth]
		s = s .. '#define ' .. mspec.name .. string.rep(' ', 39 - #mspec.name) .. ' 0x' .. mspec.id .. '\n'
		if mspec.message ~= '' then
			s = s .. '\nstruct ' .. mspec.message .. '\n{\n\tULONG MethodID;\n};\n\n'
		end
	end

	s = s .. '#endif      /* CLASSES_' .. incdef .. '_H */\n'
	fh:write(s)
	io.close(fh)
end

------------------------------------------------------------------------------------------------

-- This must be fixed to work with variable include path

function geninclude_library_header()
	local s, inc2
	namepart = string.match(spec.name, '(.-)%..+')
	incdef = string.gsub(string.upper(namepart), '%-', '_')
	inc2 = string.gsub(string.upper(spec.incpath), '%-', '_')
	libmaker.makedir('os-include')
	libmaker.makedir('os-include/libraries')
	fh = io.open('os-include/libraries/' .. namepart .. '.h', 'w')
	s = '/*\n    ' .. spec.name .. ' definitions\n\n    Copyright © ' .. spec.copyright
	s = s .. '. All rights reserved.\n*/\n\n'
	s = s .. '#ifndef ' .. inc2 .. '_' .. incdef .. '_H\n#define ' .. inc2 .. '_' .. incdef .. '_H\n\n\n'
	s = s .. '#endif      /* ' .. inc2 .. '_' .. incdef .. '_H */\n'
	fh:write(s)
	io.close(fh)
end

------------------------------------------------------------------------------------------------

function generate_includes()
	local namepart, fspec, prefix, incdef, bias

	if spec.doclass then geninclude_class_header() else geninclude_library_header() end

	namepart = string.match(spec.name, '(.+)%..-')
	libmaker.makedir('os-include')
	libmaker.makedir('os-include/fd')
	libmaker.makedir('os-include/clib')
	libmaker.makedir('os-include/inline')
	libmaker.makedir('os-include/proto')

	bias = 30
	if spec.boopsi then bias = 36 end

	-- fd --

	fh = io.open('os-include/fd/' .. namepart .. '_lib.fd', 'w')
	fh:write('##base _' .. spec.basename .. '\n##bias ' .. bias .. '\n##public\n')

	for func = 1, #spec.functions do
		fspec = spec.functions[func]
		fh:write(fspec.name)

		if #fspec.args == 0 then fh:write('(')
		else
			for arg = 1, #fspec.args do
				if arg == 1 then prefix = '(' else prefix = ',' end
				fh:write(prefix .. fspec.args[arg].name)
			end
		end

		fh:write(')(')

		for arg = 1, #fspec.args do
			if arg == 1 then prefix = '' else prefix = ',' end
			fh:write(prefix .. regnames[fspec.args[arg].m68kreg + 1])
		end

		fh:write(')\n')
	end

	fh:write('##end\n')
	io.close(fh)

	-- clib --

	fh = io.open('os-include/clib/' .. namepart .. '_protos.h', 'w')
	incdef = string.upper(namepart)
	incdef = string.gsub(incdef, '%-', '_')
	t = 'CLIB_' .. incdef .. '_PROTOS_H'
	fh:write('#ifndef ' .. t .. '\n#define ' .. t .. '\n\n')
	fh:write('/*\n   ' .. spec.name .. ' C prototypes\n   Copyright © ' .. spec.copyright .. '\n')
	fh:write('   Generated with ' .. generator .. '.\n*/\n\n\n')
	fh:write('#ifdef __cplusplus\nextern "C" {\n#endif /* __cplusplus */\n\n')

	for func = 1, #spec.functions do
		fspec = spec.functions[func]
		fh:write(fspec.type .. ' ' .. fspec.name)

		if #fspec.args == 0 then fh:write('(VOID')
		else
			for arg = 1, #fspec.args do
				if arg == 1 then prefix = '(' else prefix = ', ' end
				fh:write(prefix .. fspec.args[arg].type)
			end
		end

		fh:write(');\n')
	end

	fh:write('\n#ifdef __cplusplus\n}\n#endif /* __cplusplus */\n\n')
	fh:write('#endif /* ' .. t .. ' */\n')
	io.close(fh)

	-- GCC inline --

	fh = io.open('os-include/inline/' .. namepart .. '.h', 'w')
	t = '_INLINE_' .. incdef .. '_H'
	fh:write('#ifndef ' .. t .. '\n#define ' .. t .. '\n\n')
	fh:write('#ifndef __INLINE_MACROS_H\n#include <inline/macros.h>\n#endif\n\n')
	fh:write('#ifndef LIBRARIES_' .. incdef .. '_H\n#include <libraries/' .. namepart .. '.h>\n#endif\n\n')
	fh:write('#ifndef ' .. incdef .. '_BASE_NAME\n#define ' .. incdef .. '_BASE_NAME ' .. spec.basename .. '\n#endif\n\n')

	for func = 1, #spec.functions do
		local no_return = false
		fspec = spec.functions[func]
		if fspec.type == 'void' or fspec.type == 'VOID' then no_return = true end
		fh:write('#define ' .. fspec.name .. '(')
		for arg = 1, #fspec.args do
			if arg == 1 then fh:write(fspec.args[arg].name)
			else fh:write(', ' .. fspec.args[arg].name) end
		end
		fh:write(') \\\n\tLP' .. #fspec.args)
		if no_return then fh:write('NR') end
		fh:write('(0x' .. string.format('%02X', bias) .. ', ')
		if not no_return then fh:write(fspec.type .. ', ') end
		fh:write(fspec.name)
		for arg = 1, #fspec.args do
			fh:write(', ' .. fspec.args[arg].type .. ', ' .. fspec.args[arg].name .. ', ' .. regnames[fspec.args[arg].m68kreg + 1])
		end
		fh:write(', \\\n\t, ' .. incdef .. '_BASE_NAME)\n\n')
		bias = bias + 6
	end

	fh:write('#endif /* ' .. t .. ' */\n')
	io.close(fh)

	-- proto --

	fh = io.open('os-include/proto/' .. namepart .. '.h', 'w')
	t = '_PROTO_' .. incdef .. '_H'
	fh:write('#ifndef ' .. t .. '\n#define ' .. t .. '\n\n')
	fh:write('#ifndef EXEC_TYPES_H\n#include <exec/types.h>\n#endif\n\n')
	fh:write('#if !defined(CLIB_' .. incdef .. '_PROTOS_H) && !defined(__GNUC__)\n#include <clib/' .. namepart .. '_protos.h>\n#endif\n\n')
	fh:write('#ifndef LIBRARIES_' .. incdef .. '_H\n#include <libraries/' .. namepart .. '.h>\n#endif\n\n')
	fh:write('#ifndef __NOLIBBASE__\nextern struct Library *' .. spec.basename .. ';\n#endif\n\n')
	fh:write('#ifdef __GNUC__\n#include <inline/' .. namepart .. '.h>\n#elif !defined(__VBCC__)\n')
	fh:write('#include <pragma/' .. namepart .. '_lib.h>\n#endif\n\n')
	fh:write('#endif /* ' .. t .. ' */\n')
	io.close(fh)
end



------------------------------------------------------------------------------------------------
-- Call point
------------------------------------------------------------------------------------------------

template_preparator()
generate_simple('dummy.c', 'dummy.c', true)
generate_simple('lib_version.h', 'lib_version.h', true)
generate_simple('makefile', 'makefile', true)

if spec.boopsi then
	generate_simple('library_boopsi.c', 'library.c', true)
	generate_simple('library_boopsi.h', 'library.h', false)
else
	generate_simple('library_plain.c', 'library.c', true)
	generate_simple('library_plain.h', 'library.h', false)
end

generate_includes()
generate_functions()
if spec.doclass then generate_methods() end
