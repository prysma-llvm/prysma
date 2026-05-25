use crate::ffi::*;
use crate::*;

#[cfg(test)]
mod tests {
    use alloc::vec;
    use super::*;

    #[test]
    fn test_1() {
        let (steps, _) = schedule("2 + 3 * 4").unwrap();
        assert_eq!(steps[0].op, "*");
        assert_eq!(steps[0].result, 12.0);
        assert_eq!(steps[1].op, "+");
        assert_eq!(steps[1].result, 14.0);
    }

    #[test]
    fn test_2() {
        let (steps, _) = schedule("2 ^ 3 + 1").unwrap();
        assert_eq!(steps[0].op, "^");
        assert_eq!(steps[1].op, "+");
    }

    #[test]
    fn test_3() {
        let (_, root) = schedule("(1 + 2) * (3 + 4)").unwrap();
        assert_eq!(root, 21.0);
    }

    #[test]
    fn test_4() {
        let (steps, root) = schedule("1 + 2").unwrap();
        let s = norm_equation(&steps, root);
        assert!(s.contains("step 1:"));
        assert!(s.contains("result: 3"));
    }

    #[test]
    fn test_5() {
        let expr = (0..90).map(|_| "2").collect::<Vec<_>>().join(" ^ ");
        let (_, root) = schedule(&expr).unwrap();
        assert!(root.is_finite() || root.is_infinite());
    }

    #[test]
    fn test_6() {
        let expr = (0..90).map(|_| "1").collect::<Vec<_>>().join(" ^ ");
        let (_, root) = schedule(&expr).unwrap();
        assert_eq!(root, 1.0);
    }

    #[test]
    fn test_7() {
        let inner = "1 + 2 * 3 ^ 4 - 5 / 6 + 7 * 8";
        let expr = format!("(((({inner} + ({inner} - ({inner})");
        let err = schedule(&expr).unwrap_err();
        assert!(err.contains("Unclosed parenthesis"));
    }

    #[test]
    fn test_8() {
        let expr = "1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 @ 12";
        let err = schedule(expr).unwrap_err();
        assert!(err.contains("unknown char '@'"));
    }

    #[test]
    fn test_9() {
        let err = schedule_with_ctx(
            "1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + ghost",
            &Context::new()
        ).unwrap_err();
        assert!(err.contains("ghost"));
    }

    #[test]
    fn test_10() {
        let letters: Vec<&str> = vec![
            "a","b","c","d","e","f","g","h","i","j","k","l","m",
            "n","o","p","q","r","s","t","u","v","w","x","y","z"
        ];
        let expr = letters.join(" + ");
        let mut ctx = Context::new();
        for (i, l) in letters.iter().enumerate() {
            ctx.insert(l.to_string(), (i + 1) as f64);
        }
        let (_, root) = schedule_with_ctx(&expr, &ctx).unwrap();
        assert_eq!(root, 351.0);
    }

    #[test]
    fn test_11() {
        let letters: Vec<&str> = vec![
            "a","b","c","d","e","f","g","h","i","j","k","l","m",
            "n","o","p","q","r","s","t","u","v","w","x","y","z"
        ];
        let expr = letters.join(" * ");
        let mut ctx = Context::new();
        for l in &letters {
            ctx.insert(l.to_string(), 2.0);
        }
        let (_, root) = schedule_with_ctx(&expr, &ctx).unwrap();
        assert_eq!(root, 2_f64.powi(26));
    }

    #[test]
    fn test_12() {
        let open  = "(".repeat(90);
        let close = ")".repeat(90);
        let expr  = format!("{open}2 + 3{close}");
        let (_, root) = schedule(&expr).unwrap();
        assert_eq!(root, 5.0);
    }

    #[test]
    fn test_13() {
        let mut expr = "2 ^ 3".to_string();
        for _ in 0..90 {
            expr = format!("({expr} ^ 2)");
        }
        let (_, root) = schedule(&expr).unwrap();
        assert!(root.is_finite() || root.is_infinite());
    }

    #[test]
    fn test_14() {
        let mut expr = "1".to_string();
        for i in 2..=60 {
            let op = match i % 4 {
                0 => "+", 1 => "-", 2 => "*", _ => "^",
            };
            expr = format!("({expr} {op} {i})");
        }
        let result = schedule(&expr);
        assert!(result.is_ok() || result.is_err());
    }

    #[test]
    fn test_15() {
        let (_, root) = schedule("100 * 100 - 1").unwrap();
        assert_eq!(root, 9999.0);
    }

    #[test]
    fn test_16() {
        let mut ctx = Context::new();
        ctx.insert("x".to_string(), 5.0);
        let (_, root) = schedule_with_ctx("-x + 10", &ctx).unwrap();
        assert_eq!(root, 5.0);
    }

    #[test]
    fn test_17() {
        let (_, root) = schedule("1 / 3").unwrap();
        assert!((root - 0.333_333_333_333_333_3).abs() < 1e-10);
    }

    #[test]
    fn test_18() {
        let (_, root) = schedule("3 - 10").unwrap();
        assert_eq!(root, -7.0);
    }

    #[test]
    fn test_19() {
        let (_, root) = schedule("2 ^ 3 ^ 2").unwrap();
        assert_eq!(root, 512.0);
    }

    #[test]
    fn test_20() {
        let err = schedule("").unwrap_err();
        assert!(err.contains("Error"));
    }

    #[test]
    fn test_21() {
        let (_, root) = schedule("42").unwrap();
        assert_eq!(root, 42.0);
    }

    #[test]
    fn test_22() {
        let err = schedule("(1 + @ 2 * )").unwrap_err();
        assert!(err.contains("'@'"));
    }
}
