// This file is part of the VroomJs library.
//
// Author:
//     Federico Di Gregorio <fog@initd.org>
//
// Copyright (c) 2013 
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

using System;
using System.Collections.Generic;
using System.Dynamic;

namespace VroomJs
{
    public class JsObject : DynamicObject, IDisposable
    {
        public JsObject(JsContext context, IntPtr ptr)
        {
            if (ptr == IntPtr.Zero)
                throw new ArgumentException("can't wrap an empty object (ptr is Zero)", "ptr");

			_context = context;
            _handle = ptr;
		}

		readonly JsContext _context;
        readonly IntPtr _handle;

        public IntPtr Handle {
            get { return _handle; }
        }

        public override bool TryInvokeMember(InvokeMemberBinder binder, object[] args, out object result)
        {
			result = _context.InvokeProperty(this, binder.Name, args);
            return true;
        }

		public override bool TryGetMember(GetMemberBinder binder, out object result)
        {
			result = _context.GetPropertyValue(this, binder.Name);
            return true;
        }

        public override bool TrySetMember(SetMemberBinder binder, object value)
        {
			_context.SetPropertyValue(this, binder.Name, value);
            return true;
        }

		public override IEnumerable<string> GetDynamicMemberNames() 
		{
			return _context.GetMemberNames(this);
		}

        #region IDisposable implementation

        bool _disposed;

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (_disposed)
                throw new ObjectDisposedException("JsObject:" + _handle);

            _disposed = true;

            _context.Engine.DisposeObject(this.Handle);
        }

        ~JsObject()
        {
            if (!_disposed)
                Dispose(false);
        }

        #endregion
    }
}


