[Z, M, A, HK, XY, V] = LoadData('trainingdata.txt');

%P = [linspace(0, 120, 25)' linspace(0, 120, 25)'];
%l_ = Inf;
P = P_;

[fXY, S, F] = Compute(XY, V, P);

I = 0;
c = 0;

while 1
    [DLDP] = ComputeGradient(Z, M, A, HK, XY, V, P, fXY, F);
    P = P - 0.1^5 * DLDP;
    I = I + 1;
    
    [fXY, S, F] = Compute(XY, V, P);
    % Compute Loss
    l = 0;
    for i = 1:Z
        s = ones(1, M(i)) * fXY(A(i):(A(i) + M(i) - 1), : ) - HK(i, : );
        l = l + sqrt(s * s');
    end
    l = l / Z;
    
    if l < l_
        P_ = P;
        l_ = l;
    end
    
    fprintf('I: %d, l: %d, l_: %d\n', I, l, l_);
    if I > 10000
        if l == l_
            c = 0;
        elseif c == 100
            break;
        else 
           c = c + 1;
        end
    end
end

fprintf('l_: %d\n', l_);