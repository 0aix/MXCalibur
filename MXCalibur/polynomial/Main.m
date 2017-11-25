[Z, M, A, HK, XY, V] = LoadData('trainingdata.txt');

C = [2.5957204928923185e+000; 8.3322606888998951e-001; -1.0738616803756307e-002; 4.3696740339727921e-005];
R = [1; 1366 / 768];
%R = [1; 1];
%l_ = Inf;
C = C_;
R = R_;

[fXY] = Compute(XY, V, C, R);

I = 0;
c = 0;

while 1
    [DLDC, DLDR] = ComputeGradient(Z, M, A, HK, XY, V, C, R, fXY);
    C = C - 0.1^4 * 1 * DLDC;
    R = R - 0.1^2 * 1 * DLDR;
    I = I + 1;
    
    [fXY] = Compute(XY, V, C, R);
    % Compute Loss
    l = 0;
    for i = 1:Z
        s = ones(1, M(i)) * fXY(A(i):(A(i) + M(i) - 1), : ) - HK(i, : );
        l = l + sqrt(s * s');
    end
    l = l / Z;
    
    if l < l_
        C_ = C;
        R_ = R;
        l_ = l;
    end
    
    fprintf('I: %d, l: %d, l_: %d\n', I, l, l_);
    if I > 10000
        if l == l_
            c = 0;
        elseif c == 10000
            break;
        else 
           c = c + 1;
        end
    end
end

fprintf('l_: %d\n', l_);