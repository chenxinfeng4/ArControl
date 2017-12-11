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
for i=1:length(pfnames);
    pfmats{i} = trans(pfnames{i});
end

%%
function pfnew = trans(pf)
[~, f, ~] = fileparts(pf);
pfnew = regexprep(pf, '\.txt$', '.mat');
MAT = struct();
fid = fopen(pf);
frewind(fid); %to the begin of file;
expression = '^(IN\d+|OUT\d+|C\d+S\d+):(\w.*)$';
while ~feof(fid)
    str=fgetl(fid);
    if isequal(str, -1); return; end;
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

