function pfmats = BF_arc2mat(pfnames)
%Convert ArControl .TXT to Matlab .MAT
%function BF_arc2mat(pfnames)
%-------------Input Paras---------
% pfnames    : TXT file from ArControl
%
%-------------Output Para---------
% pfmats     : file of generated MAT, cell type.
%
%--------------Example-------------
%BF_arc2mat(); %Pop up File Dialog
%BF_arc2mat({'1.txt', '2.txt'});

if ~exist('pfnames', 'var')
    pfnames = uigetfilemult('*.txt');
end
pfmats = cell(size(pfnames));
for i=1:length(pfnames)
    pfmats{i} = trans(pfnames{i});
end

if ~exist('pfnames', 'var')
    pfnames = uigetfilemult('*.txt');
end
pfmats = cell(size(pfnames));
for i=1:length(pfnames)
    pfmats{i} = trans(pfnames{i});
end

%%
function pfnew = trans(pf)
pfnew = regexprep(pf, '\.txt$', '_arc.mat');
MAT = struct();
fid = fopen(pf, 'r', 'n', 'utf-8');
frewind(fid); %to the begin of file;

% header %
expression_header = '^@(IN\d+|OUT\d+|C\d+|C\d+S\d+):(.*)$';
expression_taskname = '^-----(\w+)-----$';
expression_arcbg = '^ArC-bg$';
MAT.info = struct();
isokfile = false;
while ~feof(fid)
    str=fgetl(fid);
    if regexp(str, expression_header, 'once')
        Style = regexprep(str,expression_header,'$1');
        comment = regexprep(str,expression_header,'$2');
        MAT.info.(Style) = comment;
    elseif regexp(str, expression_taskname, 'once')
        taskname= regexprep(str,expression_taskname,'$1');
        MAT.info.task = taskname;
    elseif regexp(str,expression_arcbg, 'once')
        isokfile = true;
        break;
    end
end
assert(isokfile, 'It''s NOT a data file from ArControl!');

% data %
expression = '^(IN\d+|OUT\d+|C\d+S\d+):(\w.*)$';
while ~feof(fid)
    str=fgetl(fid);
    if isequal(str, -1); return; end
    if regexp(str,expression,'once')
        Style = regexprep(str,expression,'$1');
        Nums = regexprep(str,expression,'$2');
        if ~isfield(MAT,Style); MAT.(Style)=[]; end
        MAT.(Style) = [MAT.(Style);str2num(Nums)]; %#ok<ST2NM>
     end
end
fclose(fid);
save(pfnew, '-struct', 'MAT');


function pfname=uigetfilemult(varargin)
[fname,pname]=uigetfile(varargin{:},'MultiSelect','on');
switch class(fname)
    case 'double'%no file
        pfname={};
        return
    case 'char' %single file
        pfname={[pname,fname]};
    case 'cell' %files
        pfname=cell(size(fname));
        for i=1:length(fname)
            pfname(i)={[pname,fname{i}]};
        end

end

