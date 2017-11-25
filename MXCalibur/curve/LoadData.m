function [Z, M, A, HK, XY, V] = LoadData(file_path)
 % Load Mouse Training Data
    f = fopen(file_path);
    buffer = textscan(f, '%f', 1);
    Z = buffer{1};
    buffer = textscan(f, '%f', Z);
    M = buffer{1};
    A = ones(Z, 1);
    for i = 2:Z
       A(i) = A(i - 1) + M(i - 1);
    end
    buffer = textscan(f, '%f %f', Z);
    HK = [buffer{1} buffer{2}];
    buffer = textscan(f, '%f %f', sum(M));
    XY = [buffer{1} buffer{2}];
    V = sqrt(sum(XY .* XY, 2));
    fclose(f);
end

