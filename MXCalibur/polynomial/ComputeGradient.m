function [DLDC, DLDR] = ComputeGradient(Z, M, A, HK, XY, V, C, R, fXY)
    DLDXY = zeros(size(XY));
    for i = 1:Z
        U = A(i):(A(i) + M(i) - 1);
        dldxy = ones(1, M(i)) * fXY(U, : ) - HK(i, : );
        dldxy = (dldxy * dldxy').^-0.5 * dldxy;
        DLDXY(U,  : ) = repmat(dldxy, M(i), 1);
    end
    
    DLDR = ones(2, 1);
    DLDR(1) = sum(DLDXY( : , 1) .* fXY( : , 1) / R(1));
    DLDR(2) = sum(DLDXY( : , 2) .* fXY( : , 2) / R(2));
    DLDR = 0.1.^(floor(log10(abs(DLDR))) + 4) .* DLDR;
    DLDR = DLDR / Z;
    %DLDR(1) = 0;
    
    xy = [R(1) * XY( : , 1) R(2) * XY( : , 2)];
    DLDXY = sum(DLDXY .* xy, 2);
    
    DFDC = zeros(size(C));
    DFDC(1) = sum(DLDXY);
    DLDXY = DLDXY .* V;
    DFDC(2) = sum(DLDXY);
    DLDXY = DLDXY .* V;
    DFDC(3) = sum(DLDXY);
    DLDXY = DLDXY .* V;
    DFDC(4) = sum(DLDXY);
    DFDC = 0.1.^(floor(log10(abs(DFDC))) + 4) .* DFDC;
    DLDC = DFDC / Z;
    
    
end

