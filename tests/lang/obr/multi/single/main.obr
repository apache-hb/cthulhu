MODULE Dep;
    VAR
        a : INTEGER;

    PROCEDURE SetData (VAR value : INTEGER)
    BEGIN
        a := value;
    END SetData;

    PROCEDURE GetData (VAR value : INTEGER)
    BEGIN
        value := a;
    END GetData;
END Dep.

MODULE Main;
    IMPORT Dep, Out;

    VAR
        a : INTEGER;
BEGIN
    (* set a to 5 *)
    Dep.SetData(5);

    (* get a *)
    Dep.GetData(a);

    Out.Int(a, 0);

    IF a = 5 THEN
        Out.String("PASS");
    ELSE
        Out.String("FAIL");
    END;
END Main.
