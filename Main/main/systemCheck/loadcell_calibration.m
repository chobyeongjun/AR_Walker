function [left_slope, left_intercept, left_R2, right_slope, right_intercept, right_R2] = loadcell_calibration()
% LOADCELL_CALIBRATION - 로드셀 4점 캘리브레이션 함수 (0N 포함)
% 실제 측정된 전압값을 사용하여 선형 캘리브레이션을 수행
%
% 출력:
%   left_slope, left_intercept, left_R2   - 왼쪽 로드셀 캘리브레이션 결과
%   right_slope, right_intercept, right_R2 - 오른쪽 로드셀 캘리브레이션 결과

    % 4점 캘리브레이션 데이터 (0N 포함)
    weights_kg = [0, 2, 5, 7.5];  % 무게 (kg) - 0N 추가
    theoretical_force = weights_kg * 9.81;  % 이론적 힘 (N)
    
    % 실제 측정된 전압값 (제공된 데이터)
    left_voltage = [1.216594118, 1.25389932, 1.306053589, 1.34800566];    % 왼쪽 로드셀 전압
    right_voltage = [1.207617279, 1.245621488, 1.307152778, 1.339987013]; % 오른쪽 로드셀 전압
    
    fprintf('=== 4점 로드셀 캘리브레이션 (0N 포함) ===\n');
    fprintf('캘리브레이션 포인트:\n');
    for i = 1:length(weights_kg)
        fprintf('  %.1f kg = %.2f N\n', weights_kg(i), theoretical_force(i));
    end
    fprintf('\n');
    
    % 측정 데이터 확인
    fprintf('=== 측정 데이터 ===\n');
    fprintf('무게(kg) | 힘(N)  | 왼쪽전압(V) | 오른쪽전압(V)\n');
    fprintf('---------|--------|-------------|---------------\n');
    for i = 1:length(weights_kg)
        fprintf('  %5.1f  | %6.2f |   %9.6f |     %9.6f\n', ...
                weights_kg(i), theoretical_force(i), left_voltage(i), right_voltage(i));
    end
    fprintf('\n');
    
    %% 왼쪽 로드셀 캘리브레이션
    fprintf('=== 왼쪽 로드셀 캘리브레이션 ===\n');
    
    % 선형 회귀 수행 (Force = slope * Voltage + intercept)
    p_left = polyfit(left_voltage, theoretical_force, 1);
    left_slope = p_left(1);
    left_intercept = p_left(2);
    
    % 결정계수 계산
    force_fitted_left = polyval(p_left, left_voltage);
    SS_res_left = sum((theoretical_force - force_fitted_left).^2);
    SS_tot_left = sum((theoretical_force - mean(theoretical_force)).^2);
    left_R2 = 1 - (SS_res_left / SS_tot_left);
    
    fprintf('선형 방정식: Force = %.4f * Voltage + %.4f\n', left_slope, left_intercept);
    fprintf('기울기 (Sensitive): %.4f N/V\n', left_slope);
    fprintf('절편 (Bias): %.4f N\n', left_intercept);
    fprintf('결정계수 (R²): %.6f\n', left_R2);
    
    %% 오른쪽 로드셀 캘리브레이션
    fprintf('\n=== 오른쪽 로드셀 캘리브레이션 ===\n');
    
    % 선형 회귀 수행
    p_right = polyfit(right_voltage, theoretical_force, 1);
    right_slope = p_right(1);
    right_intercept = p_right(2);
    
    % 결정계수 계산
    force_fitted_right = polyval(p_right, right_voltage);
    SS_res_right = sum((theoretical_force - force_fitted_right).^2);
    SS_tot_right = sum((theoretical_force - mean(theoretical_force)).^2);
    right_R2 = 1 - (SS_res_right / SS_tot_right);
    
    fprintf('선형 방정식: Force = %.4f * Voltage + %.4f\n', right_slope, right_intercept);
    fprintf('기울기 (Sensitive): %.4f N/V\n', right_slope);
    fprintf('절편 (Bias): %.4f N\n', right_intercept);
    fprintf('결정계수 (R²): %.6f\n', right_R2);
    
    %% 기존값과 비교
    fprintf('\n=== 기존 캘리브레이션 값과 비교 ===\n');
    fprintf('구분        | 기존값      | 새로운 값    | 변화량\n');
    fprintf('------------|-------------|--------------|----------\n');
    fprintf('왼쪽 Slope  | %10.4f  | %11.4f  | %+8.1f%%\n', 573.0306, left_slope, (left_slope-573.0306)/573.0306*100);
    fprintf('왼쪽 Bias   | %10.4f  | %11.4f  | %+8.1f%%\n', -699.0432, left_intercept, (left_intercept-(-699.0432))/abs(-699.0432)*100);
    fprintf('오른쪽 Slope| %10.4f  | %11.4f  | %+8.1f%%\n', 559.8251, right_slope, (right_slope-559.8251)/559.8251*100);
    fprintf('오른쪽 Bias | %10.4f  | %11.4f  | %+8.1f%%\n', -679.0051, right_intercept, (right_intercept-(-679.0051))/abs(-679.0051)*100);
    
    %% 그래프 그리기
    figure('Position', [100, 100, 1200, 500]);
    
    % 왼쪽 로드셀 그래프
    subplot(1,2,1);
    hold on;
    
    % 실제 측정값
    plot(left_voltage, theoretical_force, 'ro', 'MarkerSize', 10, 'LineWidth', 2);
    
    % 선형 피팅 라인
    voltage_range_left = linspace(min(left_voltage)*0.98, max(left_voltage)*1.02, 100);
    force_fitted_range_left = polyval(p_left, voltage_range_left);
    plot(voltage_range_left, force_fitted_range_left, 'r-', 'LineWidth', 2);
    
    xlabel('전압 (V)');
    ylabel('힘 (N)');
    title(sprintf('왼쪽 로드셀 캘리브레이션 (R² = %.4f)', left_R2));
    legend({'측정값', sprintf('y = %.1fx + %.1f', left_slope, left_intercept)}, 'Location', 'best');
    grid on;
    
    % 각 점에 레이블 추가
    for i = 1:length(left_voltage)
        text(left_voltage(i), theoretical_force(i), sprintf('  %.1fkg', weights_kg(i)), ...
             'VerticalAlignment', 'bottom', 'FontSize', 10);
    end
    hold off;
    
    % 오른쪽 로드셀 그래프
    subplot(1,2,2);
    hold on;
    
    % 실제 측정값
    plot(right_voltage, theoretical_force, 'bo', 'MarkerSize', 10, 'LineWidth', 2);
    
    % 선형 피팅 라인
    voltage_range_right = linspace(min(right_voltage)*0.98, max(right_voltage)*1.02, 100);
    force_fitted_range_right = polyval(p_right, voltage_range_right);
    plot(voltage_range_right, force_fitted_range_right, 'b-', 'LineWidth', 2);
    
    xlabel('전압 (V)');
    ylabel('힘 (N)');
    title(sprintf('오른쪽 로드셀 캘리브레이션 (R² = %.4f)', right_R2));
    legend({'측정값', sprintf('y = %.1fx + %.1f', right_slope, right_intercept)}, 'Location', 'best');
    grid on;
    
    % 각 점에 레이블 추가
    for i = 1:length(right_voltage)
        text(right_voltage(i), theoretical_force(i), sprintf('  %.1fkg', weights_kg(i)), ...
             'VerticalAlignment', 'bottom', 'FontSize', 10);
    end
    hold off;
    
    %% 검증 테스트
    fprintf('\n=== 왼쪽 로드셀 검증 ===\n');
    fprintf('무게(kg) | 전압(V)   | 계산힘(N) | 이론힘(N) | 오차(%%)\n');
    fprintf('---------|-----------|-----------|-----------|----------\n');
    for i = 1:length(left_voltage)
        calculated_force = left_slope * left_voltage(i) + left_intercept;
        error_percent = abs(calculated_force - theoretical_force(i)) / theoretical_force(i) * 100;
        fprintf('  %5.1f  | %8.6f  | %8.3f  | %8.3f  | %7.2f\n', ...
                weights_kg(i), left_voltage(i), calculated_force, theoretical_force(i), error_percent);
    end
    
    fprintf('\n=== 오른쪽 로드셀 검증 ===\n');
    fprintf('무게(kg) | 전압(V)   | 계산힘(N) | 이론힘(N) | 오차(%%)\n');
    fprintf('---------|-----------|-----------|-----------|----------\n');
    for i = 1:length(right_voltage)
        calculated_force = right_slope * right_voltage(i) + right_intercept;
        error_percent = abs(calculated_force - theoretical_force(i)) / theoretical_force(i) * 100;
        fprintf('  %5.1f  | %8.6f  | %8.3f  | %8.3f  | %7.2f\n', ...
                weights_kg(i), right_voltage(i), calculated_force, theoretical_force(i), error_percent);
    end
    
    %% 캘리브레이션 결과 요약
    fprintf('\n=== Arduino 코드용 캘리브레이션 값 ===\n');
    fprintf('// 왼쪽 로드셀\n');
    fprintf('float left_knee_bias = %.4ff;\n', left_intercept);
    fprintf('float left_knee_sensitive = %.4ff;\n', left_slope);
    fprintf('\n// 오른쪽 로드셀\n');
    fprintf('float right_knee_bias = %.4ff;\n', right_intercept);
    fprintf('float right_knee_sensitive = %.4ff;\n', right_slope);
    
    %% 사용법 안내
    fprintf('\n=== 사용법 ===\n');
    fprintf('왼쪽 로드셀: Force_N = %.4f * Voltage_V + (%.4f)\n', left_slope, left_intercept);
    fprintf('오른쪽 로드셀: Force_N = %.4f * Voltage_V + (%.4f)\n', right_slope, right_intercept);
    
    % 캘리브레이션 함수 저장
    save_calibration_functions(left_slope, left_intercept, right_slope, right_intercept);
    
    % 결과 저장
    save('loadcell_calibration_results.mat', 'left_slope', 'left_intercept', 'left_R2', ...
         'right_slope', 'right_intercept', 'right_R2', 'weights_kg', 'theoretical_force', ...
         'left_voltage', 'right_voltage');
    fprintf('\n캘리브레이션 결과가 "loadcell_calibration_results.mat" 파일로 저장되었습니다.\n');
end

function save_calibration_functions(left_slope, left_intercept, right_slope, right_intercept)
% 캘리브레이션 함수들을 별도 파일로 저장
    
    % 왼쪽 로드셀 함수
    filename_left = 'voltage_to_force_left.m';
    fid = fopen(filename_left, 'w');
    
    if fid ~= -1
        fprintf(fid, 'function force = voltage_to_force_left(voltage)\n');
        fprintf(fid, '%% VOLTAGE_TO_FORCE_LEFT - 왼쪽 로드셀 전압을 힘으로 변환\n');
        fprintf(fid, '%% 입력: voltage (V)\n');
        fprintf(fid, '%% 출력: force (N)\n');
        fprintf(fid, '%% 캘리브레이션 날짜: %s\n', datestr(now));
        fprintf(fid, '%%\n');
        fprintf(fid, '%% 캘리브레이션 방정식: Force = %.6f * Voltage + (%.6f)\n\n', left_slope, left_intercept);
        fprintf(fid, '    force = %.6f * voltage + (%.6f);\n', left_slope, left_intercept);
        fprintf(fid, 'end\n');
        fclose(fid);
    end
    
    % 오른쪽 로드셀 함수
    filename_right = 'voltage_to_force_right.m';
    fid = fopen(filename_right, 'w');
    
    if fid ~= -1
        fprintf(fid, 'function force = voltage_to_force_right(voltage)\n');
        fprintf(fid, '%% VOLTAGE_TO_FORCE_RIGHT - 오른쪽 로드셀 전압을 힘으로 변환\n');
        fprintf(fid, '%% 입력: voltage (V)\n');
        fprintf(fid, '%% 출력: force (N)\n');
        fprintf(fid, '%% 캘리브레이션 날짜: %s\n', datestr(now));
        fprintf(fid, '%%\n');
        fprintf(fid, '%% 캘리브레이션 방정식: Force = %.6f * Voltage + (%.6f)\n\n', right_slope, right_intercept);
        fprintf(fid, '    force = %.6f * voltage + (%.6f);\n', right_slope, right_intercept);
        fprintf(fid, 'end\n');
        fclose(fid);
    end
    
    fprintf('\n캘리브레이션 함수가 저장되었습니다:\n');
    fprintf('- %s\n', filename_left);
    fprintf('- %s\n', filename_right);
end