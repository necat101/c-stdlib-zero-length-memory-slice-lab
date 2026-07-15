# HN thread 42387013 — Making memcpy(NULL, NULL, 0) well-defined

Source: https://news.ycombinator.com/item?id=42387013
Fetched 261 items.

## gslin — 42387013



## whytevuhuni — 42387178

How interesting. GCC does indeed remove that branch.

<a href="https:&#x2F;&#x2F;godbolt.org&#x2F;z&#x2F;aPcr1bfPe" rel="nofollow">https:&#x2F;&#x2F;godbolt.org&#x2F;z&#x2F;aPcr1bfPe</a>

## voidUpdate — 42387102

I feel like I&#x27;ve misunderstood something here... shouldn&#x27;t memcpy(anything, anything, 0) just do nothing, because you&#x27;re copying 0 bytes?

## nmilo — 42393030

&gt; However, the most vocal opposition came from a static analysis perspective: Making null pointers well-defined for zero length means that static analyzers can no longer unconditionally report NULL being passed to functions like memcpy—they also need to take the length into account now.

How does this make any sense? We don&#x27;t want to remove a low hanging footgun because static analyzers can no longer detect it?

## mjg59 — 42388713

Explanation for the above: passing NULL as the destination argument to memcpy() is undefined behaviour at present. gcc assumes that the fact that memcpy() is called therefore means that the destination argument can&#x27;t be NULL, so &quot;knows&quot; that the dest == NULL check can never be true, and so removes the test and the do_thing1() branch entirely.

Interestingly, replacing len in the memcpy() call results in gcc instead removing the memcpy() call and retaining the check - presumably a different optimisation routine decides that it&#x27;s a no-op in that case. <a href="https:&#x2F;&#x2F;godbolt.org&#x2F;z&#x2F;cPdx6v13r" rel="nofollow">https:&#x2F;&#x2F;godbolt.org&#x2F;z&#x2F;cPdx6v13r</a> is, therefore, interesting - despite this only ever calling test() with a len of 0, the elision of the dest == NULL check is still there, but test() has been inlined <i>without</i> the memcpy (because len == 0) but <i>with</i> do_thing2() (because the behaviour is undefined and so it can assume dest isn&#x27;t NULL even though there&#x27;s a NULL literally right there!)

Fucking compilers, man.

## mjg59 — 42387125

That&#x27;s a reasonable intuitive interpretation of how it <i>should</i> behave, but according to the spec it&#x27;s undefined behaviour and compilers have a great degree of freedom in what happens as a result.

## pkhuong — 42387175

It does nothing, but is only defined when the pointers point into or one past the end of valid objects (live allocations), because that&#x27;s how the standard defines the C VM, in terms of objects, not a flat byte array.

## sfink — 42394514

No, it means the static analyzers can&#x27;t report on a <i>different</i> error because a subset of that class of errors is no longer an error, and the static analysis can&#x27;t usually distinguish between that subset and the rest.

<pre><code>    memcpy(NULL, NULL, 0); &#x2F;&#x2F; Formerly bad, now ok.
    memcpy(NULL, NULL, s); &#x2F;&#x2F; Formerly bad, now unknown (unless it can be proven that s != 0).</code></pre>
and

<pre><code>    memcpy(NULL, b, c); &#x2F;&#x2F; Same issue.
</code></pre>
(where &quot;NULL&quot; == &quot;statically known to be NULL&quot;, not necessarily just a literal NULL. Not that that changes the difficulty here.)

Previously: warn if either address might be NULL.

Now: warn if either address might be NULL <i>and</i> the length might be nonzero, and prepare for your users to be annoyed and shut this warning off due to the false alarms.

Any useful static analysis tool does a careful balance between false positives and false negatives (aka false alarms and missed bugs). Too many false positives, and that warning will be disabled, or users will get used to ignoring it, or it will be routinely annotated away at call sites without anyone bothering to figure out whether it&#x27;s valid or not. Soon the tool will cease to be useful and may be entirely abandoned. In actual practice, the sophistication of a static analysis tool is far less relevant than its precision. It&#x27;s quite common to have an incredibly powerful static analysis tool that is use

## int_19h — 42392992

As the article points out, all major memcpy implementations already do this check inside memcpy. Sure, the caller can also check, but given that it&#x27;s both redundant in practice and makes some common patterns harder to use than they would otherwise be, there&#x27;s no reason to not just standardize what&#x27;s already happening anyway and make everyone&#x27;s lives easier in the process.

## david-gpu — 42387419

More information on this behavior in the link below.

<i>&gt; Note that, apart from contrived examples with deleted null checks, the current rules do not actually help the compiler meaningfully optimize code. A memcpy implementation cannot rely on pointer validity to speculatively read because, even though memcpy(NULL, NULL, 0) is undefined, slices at the end of a buffer are fine. [And if the end of the buffer] were at the end of a page with nothing allocated afterwards, a speculative read from memcpy would break</i>

<a href="https:&#x2F;&#x2F;davidben.net&#x2F;2024&#x2F;01&#x2F;15&#x2F;empty-slices.html" rel="nofollow">https:&#x2F;&#x2F;davidben.net&#x2F;2024&#x2F;01&#x2F;15&#x2F;empty-slices.html</a>

## voidUpdate — 42387160

Why didn&#x27;t they just... define it, back when they wrote it?

## whytevuhuni — 42387223

What if the objects are non-NULL, but invalid (not actually allocated)?

For example, Rust will use address 1 with length 0 for static empty strings, because 1 is a properly aligned non-null pointer.

I would imagine such strings end up being passed to C code sometimes, which may end up calling memcpy with a length of 0 on them.

## comex — 42393905

Just for the record, that&#x27;s not the main purpose of -fdelete-null-pointer-checks.

Normally, it only deletes null checks after actual null pointer dereferences.  In principle this can&#x27;t change observable behavior.  Null dereferences are guaranteed to trap, so if you don&#x27;t trap, it means the pointer wasn&#x27;t null.  In other words, <i>unlike most C compiler optimizations</i>, -fdelete-null-pointer-checks should be safe even if you do commit undefined behavior.

This once caused a kerfuffle with the Linux kernel.  At the time, x86_64 CPUs allowed the kernel to dereference userspace addresses, and the kernel allowed userspace to map address 0.  Therefore, it was possible for userspace to arrange for null pointers to <i>not</i> trap when dereferenced in the kernel.  Which meant that the null check optimization could actually change observable behavior.  Which introduced a security vulnerability. [1]

Since then, Linux has been compiled with `-fno-delete-null-pointer-checks`, but it&#x27;s not really necessary: Linux systems have long since enforced that userspace can&#x27;t map address 0, which means that deleting null pointer checks should be safe in both kernel and userspace.  (Newer CPU security features also protect the kernel even if userspace <i>is</i> allowed to map address 0.)

But anyway, I didn&#x27;t know that -fdelete-null-pointer-checks treated &quot;memcpy with potentially-zero size&quot; as a condition to remove subsequent null pointer checks.  Tha

## mjg59 — 42390661

The valid inputs to memcpy() are defined by the C specification, so the compiler is free to make assumptions about what valid inputs are even if the library implementation chooses to allow a broader range of inputs

## int_19h — 42392576

Per ISO C, the identifiers declared or defined with external linkage by any C standard library header are considered reserved, so the moment you define your own memcpy, you&#x27;re already in UB land.

## bonzini — 42393013

If you do so you have to add -fno-builtins (or just -fno-builtin-memcpy).

## bonzini — 42393072

In real mode assembly days, ES and sometimes DS were just another base register that you could use in a loop. Given the dearth of addressing modes it was quite nice to assume that large arrays started at xxxx0h and therefore that the offset part of the far pointer was zero.

## int_19h — 42392818

People will only rely on UB when it is well defined by a particular implementation, either explicitly or because of a long history of past use. E.g. using unions for type punning in gcc, or allowing methods to be called on null pointers in MSVC.

But there&#x27;s nothing like that here.

## pkhuong — 42387296

also UB according to the spec, but LLVM is free to define it. e.g., clang often converts trivial C++ copy constructors to memcpy, which is UB for self-assignment, but I assume that&#x27;s fine because the C++ front-end only targets LLVM, and LLVM presumably defines the behaviour to do what you&#x27;d expect.

## sfink — 42394337

&gt; Similarly, GCC may delete a memcpy to a buffer about to be freed, although I have never observed that as you generally don’t do that in production code.

It&#x27;s not that crazy. You could have a refcounted object that poisons itself when the refcount drops to zero, but doesn&#x27;t immediately free itself because many malloc implementations can have bad lock contention on free(). So you poison the object to detect bugs, possibly only in certain configurations, and then queue the pointer for deferred freeing on a single thread at a better time.

(Ok, this doesn&#x27;t quite do it: poisoning is much more likely to use memset than memcpy, but I assume gcc would optimize out a doomed memset too?)

## bonzini — 42393209

This is wrong. If you do p=malloc(256), p+256 is valid even though it does not point to a valid address (it might be in an unmapped page; check out ElectricFence). Rust&#x27;s non-null aligned other pointer is the same, memcpy can&#x27;t assume it can be dereferenced if the size is zero. The standard text in the linked paper says the same.

## whytevuhuni — 42387329

Where I work, it is quite normal to link together C code compiled with GCC and Rust code compiled with LLVM, due to how the build system is set up.

As far as I know that disables LTO, but the build system is so complex, and the C code so large, that nobody bothers switching the C side to Clang&#x2F;LLVM as well.

## bonzini — 42393025

The compiler is free to give a meaning to memcpy if run in the (default) hosted mode. There&#x27;s -ffreestanding for freestanding environments.

## bonzini — 42393164

Rep movsb copies 64K if CX=0 (that&#x27;s actually very useful), but memcpy could be implemented as two instructions:

<pre><code>    jcxz skip 
    rep movsb
    skip:</code></pre>

## david-gpu — 42389724

Yikes! I would love sipping coffee watching the chief architect chew up whoever suggested that. That sounds awful even on a microcontroller.

## bonzini — 42393133

On s390 the memory at address 0 (low core) has all sorts of important stuff. Of course s390 has paging enabled pretty much always but still...

## whytevuhuni — 42392235

I think the first one, stack overflow, is technically not a memory safety issue, just denial-of-service on resource exhaustion. Stack overflow is well defined as far as I know.

The other three are definitely memory safety issues.

## david-gpu — 42395038

Thanks for saving me a search, because I was expecting r0 to be hardcoded to zero.

Sometimes hardware is designed with insufficient input from software folks and the result is something asinine like that. That, or some people like watching the world burn.

## whytevuhuni — 42393426

In the general case, I think you might be right, although it&#x27;s a bit mitigated by the fact that Rust does not have support for variable length arrays, alloca, or anything that uses them, in the standard library. As you said though, it&#x27;s certainly possible.

I was more referring to that specific linked advisory, which is unlikely to use either VLAs or alloca. In that case, where stack overflow would be caused by recursion, a guard frame will always be enough to catch it, and will result in a safe abort [0].

[0] <a href="https:&#x2F;&#x2F;github.com&#x2F;rust-lang&#x2F;rust&#x2F;pull&#x2F;31333" rel="nofollow">https:&#x2F;&#x2F;github.com&#x2F;rust-lang&#x2F;rust&#x2F;pull&#x2F;31333</a>

