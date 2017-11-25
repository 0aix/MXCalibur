function [Z, M, A, N, HK, X_train, D] = LoadData(file_path)
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
    N = sum(M);
    buffer = textscan(f, '%f %f', Z);
    HK = [buffer{1} buffer{2}];
    buffer = textscan(f, '%f %f', N);
    X_train = [buffer{1} buffer{2}];
    %X_train = fXY;
    %prev = circshift([buffer{1} buffer{2}], 1);
    %for i = 1:Z
    %   prev(A(i), : ) = [0 0]; 
    %end
    %X_train = [buffer{1} buffer{2} prev];
    %D = ((HK .* HK) * [1; 1]) .^ -0.5;
    D = ones(Z, 1);
    D = D / sum(D);
    fclose(f);
end