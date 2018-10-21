
<xsl:transform
  xmlns:def='#'
  xmlns:xsl='http://www.w3.org/1999/XSL/Transform'
  xmlns:str="http://exslt.org/strings"
  extension-element-prefixes="str"
  exclude-result-prefixes='def'
  version='1.0'
>

  <!--===============================================================
    Output options
  ================================================================-->
  <xsl:output
    encoding='utf-8'
  />

  <!--===============================================================
    Rename map
  ================================================================-->
  <def:define name='rename'>

    <def:item key='rulelist' value='grammar' />
    <def:item key='elements' value='group' />
    <def:item key='element' value='group' />
    <def:item key='group' value='group' />
    <def:item key='alternation' value='choice' />
    <def:item key='concatenation' value='group' />
    <def:item key='optional' value='option' />

  </def:define>

  <!--===============================================================
    ABNF N*M repetitions
  ================================================================-->
  <xsl:template match='repetition'>
    
    <repetition>

      <xsl:variable name='repeat' select='
        normalize-space(str:concat(./repeat//_/@data))
      ' />

      <xsl:variable name='min' select='
        substring-before($repeat, "*")
      ' />

      <xsl:variable name='max' select='
        substring-after($repeat, "*")
      ' />

      <xsl:choose>

        <xsl:when test='$repeat = ""'>
          <xsl:attribute name='min'>
            <xsl:text>1</xsl:text>
          </xsl:attribute>
          <xsl:attribute name='max'>
            <xsl:text>1</xsl:text>
          </xsl:attribute>
        </xsl:when>

        <xsl:when test='not(contains($repeat, "*"))'>
          <xsl:attribute name='min'>
            <xsl:value-of select='$min'/>
          </xsl:attribute>
          <xsl:attribute name='max'>
            <xsl:value-of select='$min'/>
          </xsl:attribute>
        </xsl:when>

        <xsl:otherwise>
          <xsl:if test='string-length($min)'>
            <xsl:attribute name='min'>
              <xsl:value-of select='$min'/>
            </xsl:attribute>
          </xsl:if>
          <xsl:if test='string-length($max)'>
            <xsl:attribute name='max'>
              <xsl:value-of select='$max'/>
            </xsl:attribute>
          </xsl:if>
        </xsl:otherwise>

      </xsl:choose>

      <xsl:apply-templates/>

    </repetition>

  </xsl:template>

  <!--===============================================================
    References to non-terminals
  ================================================================-->
  <xsl:template match='element/rulename'>
    <ref name='{str:concat(.//_/@data)}' />
  </xsl:template>

  <!--===============================================================
    Comments to comments
  ================================================================-->
  <xsl:template match='comment'>
<!--
    <xsl:comment>
      <xsl:value-of select='
        substring(.//_/@data, 2)
      '/>
    </xsl:comment>
-->
  </xsl:template>

  <!--===============================================================
    String literals
  ================================================================-->
  <xsl:template match='char-val'>
    <xsl:variable name='string' select='
      substring(str:concat(.//_/@data), 2)
    '/>
    <asciiInsensitiveString text='{
      substring($string, 1, string-length($string)-1)
    }'/>
  </xsl:template>

  <!--===============================================================
    Rules
  ================================================================-->
  <xsl:template match='rule'>
    <define name='{str:concat(rulename//_/@data)}'>
      <!--
        Special handling for `foo =/ bar` rules
      -->
      <xsl:choose>
        <xsl:when test='
          "=/"
          =
          normalize-space(str:concat(defined-as//_/@data))
        '>
          <xsl:attribute name='combine'>
            <xsl:text>choice</xsl:text>
          </xsl:attribute>
        </xsl:when>
      </xsl:choose>
      <xsl:apply-templates/>
    </define>
  </xsl:template>

  <!--===============================================================
    Copy/Rename elements
  ================================================================-->
  <xsl:template match='
    rulelist
    |
    elements
    |
    element
    |
    alternation
    |
    concatenation
    |
    group
    |
    option
  '>

    <xsl:variable name='self' select='.' />

    <xsl:variable name='found' select='
      document("")
      //
      def:define[ @name = "rename" ]
      /
      def:item[ @key = local-name($self) ]
      /
      @value
    ' />

    <xsl:variable name='name'>
      <xsl:choose>
        <xsl:when test='string-length($found)'>
          <xsl:value-of select='$found' />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select='local-name($self)' />
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:element name='{$name}'>
      <xsl:apply-templates/>
    </xsl:element>
  </xsl:template>

  <!--===============================================================
    ...
  ================================================================-->

  <xsl:template match='num-val'>

    <xsl:variable name='val' select='
      str:concat(descendant::_/@data)
    ' />

    <xsl:variable name='format' select='substring($val, 2, 1)' />
    <xsl:variable name='s' select='substring($val, 3)' />

    <xsl:choose>
      <xsl:when test='contains($s, "-")'>
        <range>
          <!-- single range -->
          <xsl:attribute name='first'>
            <xsl:call-template name='num-to-decimal'>
              <xsl:with-param name='s'
                select='substring-before($s, "-")' />
              <xsl:with-param name='format' select='$format' />
            </xsl:call-template>
          </xsl:attribute>

          <xsl:attribute name='last'>
            <xsl:call-template name='num-to-decimal'>
              <xsl:with-param name='s'
                select='substring-after($s, "-")' />
              <xsl:with-param name='format' select='$format' />
            </xsl:call-template>
          </xsl:attribute>
        </range>
      </xsl:when>
      <xsl:otherwise>
      <otherwise val='{$val}' format='{$format}'>
        <!-- .-separated list  -->
        <xsl:call-template name='split-at-dot'>
          <xsl:with-param name='s' select='$s' />
          <xsl:with-param name='format' select='$format' />
        </xsl:call-template>
      </otherwise>
      </xsl:otherwise>
    </xsl:choose>

  </xsl:template>

  <!--===============================================================
    ...
  ================================================================-->
  <xsl:template name='num-to-decimal'>

    <xsl:param name='s' />
    <xsl:param name='format'/>

    <xsl:choose>
      <xsl:when test='$format = "x" or $format = "X"'>
        <xsl:value-of select='concat("U+", $s)' />
      </xsl:when>
      <xsl:when test='$format = "d" or $format = "D"'>
        <xsl:value-of select='$s' />
      </xsl:when>
      <xsl:otherwise>
        <xsl:message terminate='yes'>
          <xsl:text>Unsupported number format (</xsl:text>
          <xsl:value-of select='$format'/>
          <xsl:text>)</xsl:text>
        </xsl:message>
      </xsl:otherwise>
    </xsl:choose>

  </xsl:template>

  <!--===============================================================
    ABNF num-vals can have the form %Fa.b.c... like `CRLF = %x0D.0A`
  ================================================================-->
  <xsl:template name='split-at-dot'>
    <xsl:param name='s' />
    <xsl:param name='format'/>
    <xsl:variable name='b' select='
      substring-before(concat($s, "."), ".")
    '/>
    <xsl:variable name='a' select='
      substring-after($s, ".")'
    />

    <xsl:if test='string-length($b)'>

      <xsl:variable name='decimal'>
        <xsl:call-template name='num-to-decimal'>
          <xsl:with-param name='s' select='$b' />
          <xsl:with-param name='format' select='$format' />
        </xsl:call-template>
      </xsl:variable>

      <range first='{$decimal}' last='{$decimal}' />

    </xsl:if>
    
    <xsl:if test='string-length($a)'>
      <xsl:call-template name='split-at-dot'>
        <xsl:with-param name='s' select='$a' />
        <xsl:with-param name='format' select='$format' />
      </xsl:call-template>
    </xsl:if>

  </xsl:template>

  <!--===============================================================
    Constructs handled as part of other templates
  ================================================================-->
  <xsl:template match='_|defined-as|rule/rulename|repeat'/>

  <!--===============================================================
    Skip over
  ================================================================-->
  <xsl:template match='c-wsp|c-nl|WSP|CR|LF|SP|CRLF'>
    <xsl:apply-templates select='*' />
  </xsl:template>

  <!--===============================================================
    Unsupported constructs
  ================================================================-->
  <xsl:template match='bin-val|prose-val|*'>

    <xsl:message terminate='no'>
      <xsl:text>ERROR: </xsl:text>
      <xsl:value-of select='local-name(.)' />
      <xsl:text> values are not supported (rule: </xsl:text>
      <xsl:value-of select='str:concat(ancestor::rule/rulename//_/@data)' />
      <xsl:text>)</xsl:text>
    </xsl:message>

    <!-- When on the RHS of a rule, generate notAllowed for self -->
    <xsl:if test='ancestor::rule/elements'>
      <notAllowed>
        <xsl:comment>
          <xsl:text>Was: </xsl:text>
          <xsl:value-of select='local-name(.)' />
        </xsl:comment>
      </notAllowed>
    </xsl:if>

  </xsl:template>

</xsl:transform>
