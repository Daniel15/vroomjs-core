﻿
using System;
using System.Diagnostics;

namespace Sandbox
{
    public class NetObject
    {
        public byte Byte { get; set; }

        public bool Bool { get; set; }

        public int? Nullable { get; set; }

        public string String { get; set; }

        public int Integer { get; set; }

        public Int16 Int16 { get; set; }

        public UInt16 UInt16 { get; set; }

        public Int32 Int32 { get; set; }

        public UInt32 UInt32 { get; set; }

        public Int64 Int64 { get; set; }

        public UInt64 UInt64 { get; set; }

        public Single Single { get; set; }

        public float Float { get; set; }

        public Double Double { get; set; }

        public Decimal Decimal { get; set; }

        public DateTime Date { get; set; }

        public NetObjectNested NestedObject { get; set; }
    }

    public class NetObjectNested : NetObject
    {

    }
    
    public class NetObjectWithMethod
    {
        public int Value { get; set; }

        public void PrintValue(string msg)
        {
            Console.WriteLine(msg + " " + Value);
        }
    }

    public class Program
    {
        public static void Main(string[] args)
        {
            VroomJs.AssemblyLoader.EnsureLoaded();

            try
            {

                while (true)
                {
                    GC.Collect(GC.MaxGeneration);
                    //using (var engine = new VroomJs.JsEngine())
                    //{
                    //    using (var context = engine.CreateContext())
                    //    {
                    //        var o = new NetObject();
                    //        o.Byte = 10;
                    //        o.Bool = true;
                    //        o.Nullable = 10;
                    //        o.String = "stringvalue";
                    //        o.Integer = -53;
                    //        o.Int16 = -235;
                    //        o.UInt16 = 234;
                    //        o.Int32 = -95;
                    //        o.UInt32 = 23;
                    //        o.Int64 = -65;
                    //        o.UInt64 = 11;
                    //        o.Single = 0434.3F;
                    //        o.Float = 234.345F;
                    //        o.Double = 25D;
                    //        o.Decimal = 345;
                    //        o.Date = new DateTime(1999, 2, 2);
                    //        o.NestedObject = new NetObjectNested
                    //        {
                    //            String = "testnested"
                    //        };
                    //        context.SetVariable("test", o);

                    //        Debug.Assert((int)context.Execute("test.Byte") == 10);
                    //        Debug.Assert((bool)context.Execute("test.Bool") == true);
                    //        Debug.Assert((int)context.Execute("test.Nullable") == 10);
                    //        o.Nullable = null;
                    //        Debug.Assert(context.Execute("test.Nullable") == null);
                    //        Debug.Assert((string)context.Execute("test.String") == "stringvalue");
                    //        Debug.Assert((int)context.Execute("test.Integer") == -53);
                    //        Debug.Assert((int)context.Execute("test.Int16") == -235);
                    //        Debug.Assert((int)context.Execute("test.UInt16") == 234);
                    //        Debug.Assert((int)context.Execute("test.Int32") == -95);
                    //        Debug.Assert((int)context.Execute("test.UInt32") == 23);
                    //        Debug.Assert((int)context.Execute("test.Int64") == -65);
                    //        Debug.Assert((int)context.Execute("test.UInt64") == 11);
                    //        Debug.Assert((double)context.Execute("test.Single") == 0434.3F);
                    //        Debug.Assert((double)context.Execute("test.Float") == 234.345F);
                    //        Debug.Assert((int)context.Execute("test.Double") == 25D);
                    //        Debug.Assert((int)context.Execute("test.Decimal") == 345);
                    //        Debug.Assert(((DateTime)context.Execute("test.Date")).Equals(new DateTime(1999, 2, 2)));
                    //        Debug.Assert((string)context.Execute("test.NestedObject.String") == "testnested");
                    //    }
                    //}
                    //using (var engine = new VroomJs.JsEngine())
                    //{
                    //    using (var context = engine.CreateContext())
                    //    {
                    //        context.Execute("var t = {integer: 3}");
                    //        var r = (dynamic)context.Execute("t");
                    //        int i = r.integer;
                    //        Debug.Assert(i == 3);
                    //    }
                    //}
                    using (var engine = new VroomJs.JsEngine())
                    {
                        using (var context = engine.CreateContext())
                        {
                            context.SetVariable("m", new NetObjectWithMethod());
                            // Sets the property from Javascript.
                            context.Execute("m.Value = 42");
                            // Call a method on the CLR object from Javascript. This prints:
                            // "And the answer is (again!): 42"
                            context.Execute("m.PrintValue('And the answer is (again!):')");
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }
    }
}
